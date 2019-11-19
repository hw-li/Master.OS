/*
 * Timer.c
 *
 *  Created on: 2019��4��29��
 *      Author: Master.HE
 */
#include "Define.h"
#include "Error.h"

#include "Timer.h"

#include "__Sys.API.h"

#include "Timer.Enum.h"

#include "API.h"

#include "IRQ/IRQ.h"

#include "Scheduling/Scheduling.h"

Timer_DATA_Type Timer_DATA;

int Timer_Init(Machine_Desc_Timer_Type *P_Timer)
{
	if(P_Timer==Null)
	{
		return Error_Invalid_Parameter;
	}
	Timer_DATA.Timer=P_Timer;

	if(Timer_DATA.Timer->Init==Null
	|| Timer_DATA.Timer->GET_Flag==Null
	|| Timer_DATA.Timer->Enable==Null
	|| Timer_DATA.Timer->Disable==Null)
	{
		return Error_Undefined;
	}

	//��ʼ����ʱ��
	int Err=Timer_DATA.Timer->Init();

	if(Err!=Error_OK)
	{
		return Err;
	}

	//ע�ᶨʱ���жϺ���
	Err=__Sys_IRQ_Register_Hook(Timer_DATA.Timer->IRQ_Index,Timer_IRQ,Null);
	if(Err<Error_OK)
	{
		return Err;
	}

	//ʹ�ܶ�ʱ���ж�
	Err=__Sys_IRQ_Enable(Timer_DATA.Timer->IRQ_Index);
	if(Err!=Error_OK)
	{
		return Err;
	}

	//�ȹرն�ʱ��
	Err=Timer_DATA.Timer->Disable();
	if(Err!=Error_OK)
	{
		return Err;
	}

	Timer_DATA.Enabled=Enable;

	Timer_DATA.Timer_Queue.Begin=Null;
	Timer_DATA.Timer_Queue.End=Null;

	Err=__Sys_FIFO_Queue_Create(Null,sizeof(Timer_Function_Type),Timer_FIFO_Queue_Max_Length,Null,Null);

	if(Err<Error_OK)
	{
		return Err;
	}
	Timer_DATA.FIFO_Queue=Err;

	return Error_OK;
}
int Timer_Get_Flag(void)
{
	if(Timer_DATA.Timer->GET_Flag==Null)
	{
		return Error_Undefined;
	}
	return Timer_DATA.Timer->GET_Flag();
}
int __Sys_Timer_Enable(void)
{
	if(Timer_DATA.Timer->Enable==Null)
	{
		return Error_Undefined;
	}
	return Timer_DATA.Timer->Enable();
}
int __Sys_Timer_Disable(void)
{
	if(Timer_DATA.Timer->Disable==Null)
	{
		return Error_Undefined;
	}
	return Timer_DATA.Timer->Disable();
}
void Timer_IRQ(void *Args,int IRQ_Index)
{
	if(Timer_Get_Flag()==Error_OK)
	{

		__Timer_SysTick_Entry();

		Scheduling_SysTick();
	}

	//while(1);
}
//------------------------------------------------------------------------------------------
//--API----����APIֻ������Ȩ��ģʽ����
int __Sys_Timer_Register(
		Timer_Enter_Function Timer_Function,
		void *Args)
{
	int Err;
	if(Timer_Function==Null)
	{
		return Error_Invalid_Parameter;
	}

	Err=__Sys_Apply_Handle();
	if(Err<Valid_Handle)
	{
		return Err;
	}

	Timer_Node_Type *Temp_Timer_Node;

	Temp_Timer_Node=(Timer_Node_Type *)__Sys_Memory_Malloc(sizeof(Timer_Node_Type));

	if(Temp_Timer_Node==Null)
	{
		return Error_Allocation_Memory_Failed;
	}

	Temp_Timer_Node->Handle=Err;

	Temp_Timer_Node->N_Time_Cycle=-1;
	Temp_Timer_Node->Cycle_Time_MS=-1;

	Temp_Timer_Node->Suspended_Time_MS=-1;
	Temp_Timer_Node->Now_Countdown_N_Time_Cycle=-1;

	Temp_Timer_Node->TimeOut_MS=-1;

	Temp_Timer_Node->Timer_Function.Args=Args;
	Temp_Timer_Node->Timer_Function.Timer_Function=Timer_Function;

	if((Err=Timer_Add_Timer_Queue(Temp_Timer_Node,-1))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Timer_Node);
		return Err;
	}

	return Temp_Timer_Node->Handle;
}


int __Sys_Timer_Delete(int Handle)
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}

	int Err;
	Timer_Node_Type *Delete_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Delete_Node))==Error_OK)
	{
		if(Delete_Node==Null)
		{
			return Error_Unknown;
		}
		__Sys_Memory_Free(Delete_Node);
	}
	return Err;

}

int __Sys_Timer_Start(
		int Handle,
		int32_t N_Time_Cycle,	//����
		int32_t Cycle_Time_MS,	//����
		Timer_Operation_Type Timer_Operation)
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}
	if((N_Time_Cycle<=0 && Timer_Operation==Timer_Operation_N_Time_Cycle)
	|| Cycle_Time_MS<=0
	|| Timer_Operation>=Timer_Operation_End)
	{
		return Error_Invalid_Parameter;
	}

	//����ʱ������� ������ȡ����
	int Err;
	Timer_Node_Type *Temp_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Temp_Node))!=Error_OK)
	{
		return Err;
	}
	if(Temp_Node==Null)
	{
		return Error_Unknown;
	}
	//��������
	if(Timer_Operation==Timer_Operation_N_Time_Cycle)
	{
		Temp_Node->N_Time_Cycle=N_Time_Cycle;
	}
	else
	{
		Temp_Node->N_Time_Cycle=-1;
	}
	Temp_Node->Cycle_Time_MS=Cycle_Time_MS;

	Temp_Node->Suspended_Time_MS=-1;
	Temp_Node->Now_Countdown_N_Time_Cycle=Temp_Node->N_Time_Cycle;

	if((Err=Timer_Add_Timer_Queue(Temp_Node,Cycle_Time_MS))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Node);
		return Err;
	}
	return Error_OK;

}

int __Sys_Timer_Stop(int Handle)
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}
	//����ʱ������� ������ȡ����
	int Err;
	Timer_Node_Type *Temp_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Temp_Node))!=Error_OK)
	{
		return Err;
	}
	if(Temp_Node==Null)
	{
		return Error_Unknown;
	}
	//���ò���
	Temp_Node->Suspended_Time_MS=-1;
	Temp_Node->Now_Countdown_N_Time_Cycle=-1;

	if((Err=Timer_Add_Timer_Queue(Temp_Node,-1))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Node);
		return Err;
	}
	return Error_OK;

}

int __Sys_Timer_Suspend(int Handle)//��ͣ
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}
	//����ʱ������� ������ȡ����
	int Err;
	Timer_Node_Type *Temp_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Temp_Node))!=Error_OK)
	{
		return Err;
	}
	if(Temp_Node==Null)
	{
		return Error_Unknown;
	}
	//����ʱ��
	Temp_Node->Suspended_Time_MS=Temp_Node->TimeOut_MS;

	if((Err=Timer_Add_Timer_Queue(Temp_Node,-1))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Node);
		return Err;
	}
	return Error_OK;

}

int __Sys_Timer_Resume(int Handle)//�ָ�
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}
	//����ʱ������� ������ȡ����
	int Err;
	Timer_Node_Type *Temp_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Temp_Node))!=Error_OK)
	{
		return Err;
	}
	if(Temp_Node==Null)
	{
		return Error_Unknown;
	}
	//�������Ƿ��ʼ����
	if(Temp_Node->Cycle_Time_MS<=0
	|| Temp_Node->N_Time_Cycle==0
	|| Temp_Node->Now_Countdown_N_Time_Cycle==0)
	{

		if((Err=Timer_Add_Timer_Queue(Temp_Node,-1))!=Error_OK)
		{
			__Sys_Memory_Free(Temp_Node);
			return Err;
		}

		return Error_Operation_Failed;
	}

	//ʱ��
	int32_t TimeOut_MS=Temp_Node->Suspended_Time_MS;
	Temp_Node->Suspended_Time_MS=-1;
	if(TimeOut_MS<0)
	{
		if(Temp_Node->TimeOut_MS<0)
		{
			TimeOut_MS=Temp_Node->Cycle_Time_MS;
		}
		else
		{
			TimeOut_MS=Temp_Node->TimeOut_MS;
		}
	}

	if((Err=Timer_Add_Timer_Queue(Temp_Node,TimeOut_MS))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Node);
		return Err;
	}
	return Error_OK;

}

int __Sys_Timer_Reset(int Handle)//��λ
{
	if(Handle<Valid_Handle)
	{
		return Error_Invalid_Handle;
	}
	//����ʱ������� ������ȡ����
	int Err;
	Timer_Node_Type *Temp_Node=Null;
	if((Err=Timer_Delete_Timer_Queue(Handle,&Temp_Node))!=Error_OK)
	{
		return Err;
	}
	if(Temp_Node==Null)
	{
		return Error_Unknown;
	}
	//�������Ƿ��ʼ����
	if(Temp_Node->Cycle_Time_MS<=0
	|| Temp_Node->N_Time_Cycle==0)
	{

		if((Err=Timer_Add_Timer_Queue(Temp_Node,-1))!=Error_OK)
		{
			__Sys_Memory_Free(Temp_Node);
			return Err;
		}

		return Error_Operation_Failed;
	}

	//ʱ��

	Temp_Node->Suspended_Time_MS=-1;
	Temp_Node->Now_Countdown_N_Time_Cycle=Temp_Node->N_Time_Cycle;


	if((Err=Timer_Add_Timer_Queue(Temp_Node,Temp_Node->Cycle_Time_MS))!=Error_OK)
	{
		__Sys_Memory_Free(Temp_Node);
		return Err;
	}
	return Error_OK;

}

int __Sys_Timer_Enabled(uint8_t Enabled)
{
	if(Enabled>=Enabled_End)
	{
		return Error_Invalid_Parameter;
	}
	Timer_DATA.Enabled=(Enabled_Type)Enabled;

	return Error_OK;

}
//--End-API---

int Timer_Add_Timer_Queue(Timer_Node_Type *Add_Node,int32_t TimeOut_MS)
{
	if(Add_Node==Null || TimeOut_MS<(-1))
	{
		return Error_Invalid_Parameter;
	}
	if(TimeOut_MS<0)
	{
		if(Timer_DATA.Timer_Queue.End==Null)
		{
			Timer_DATA.Timer_Queue.Begin=Add_Node;
		}
		else
		{
			Timer_DATA.Timer_Queue.End->NEXT=Add_Node;
		}
		Timer_DATA.Timer_Queue.End=Add_Node;
		Add_Node->NEXT=Null;
		Add_Node->TimeOut_MS=-1;

		return Error_OK;
	}

	Timer_Node_Type *Temp_Node=Null,*Temp_Node_LAST=Null,*Temp_Node_NEXT=Null;
	int32_t Time_MS_NEXT=-1;


	Temp_Node=Timer_DATA.Timer_Queue.Begin;


	while(Temp_Node!=Null)
	{
		if(Temp_Node->TimeOut_MS<0)break;

		TimeOut_MS=TimeOut_MS-Temp_Node->TimeOut_MS;

		if(TimeOut_MS<0)
		{

			Temp_Node_NEXT=Temp_Node;

			TimeOut_MS=TimeOut_MS+Temp_Node->TimeOut_MS;

			Time_MS_NEXT=Temp_Node_NEXT->TimeOut_MS-TimeOut_MS;

			break;
		}
		else if(TimeOut_MS==0)
		{
			Temp_Node_LAST=Temp_Node;
			Temp_Node_NEXT=Temp_Node->NEXT;

			TimeOut_MS=0;

			Time_MS_NEXT=-1;

			break;
		}
		else //Time_MS>0
		{
			Temp_Node_LAST=Temp_Node;

			Temp_Node=Temp_Node->NEXT;
		}

	}


	if(Temp_Node_LAST==Null)//������ͷ ���ߵ�һ��
	{

		if(Timer_DATA.Timer_Queue.Begin==Null)//����Ϊ�� ��һ�δ���
		{
			Timer_DATA.Timer_Queue.Begin=Add_Node;


			Timer_DATA.Timer_Queue.End=Add_Node;
			Add_Node->NEXT=Null;

			Add_Node->TimeOut_MS=TimeOut_MS;

		}
		else if(Timer_DATA.Timer_Queue.Begin==Temp_Node_NEXT)//������ͷ֮ǰ
		{

			if(Time_MS_NEXT!=-1)
			{
				Temp_Node_NEXT->TimeOut_MS=Time_MS_NEXT;
			}

			Timer_DATA.Timer_Queue.Begin=Add_Node;


			Add_Node->NEXT=Temp_Node_NEXT;
			Add_Node->TimeOut_MS=TimeOut_MS;

		}
		else//������ͷ֮ǰ ����ͷ�����޵ȴ�
		{
			Add_Node->NEXT=Timer_DATA.Timer_Queue.Begin;

			Timer_DATA.Timer_Queue.Begin=Add_Node;

			Add_Node->TimeOut_MS=TimeOut_MS;

		}
	}
	else//�������м������β
	{
		if(Temp_Node_NEXT!=Null)//�������м�
		{
			Temp_Node_LAST->NEXT=Add_Node;


			Add_Node->TimeOut_MS=TimeOut_MS;
			Add_Node->NEXT=Temp_Node_NEXT;



			if(Time_MS_NEXT!=-1)
			{
				Temp_Node_NEXT->TimeOut_MS=Time_MS_NEXT;
			}
		}
		else//������β
		{
			if(Temp_Node_LAST==Timer_DATA.Timer_Queue.End)
			{
				Temp_Node_LAST->NEXT=Add_Node;

				Add_Node->TimeOut_MS=TimeOut_MS;
				Add_Node->NEXT=Null;

				Timer_DATA.Timer_Queue.End=Add_Node;
			}
			else//������β֮ǰ β��ʱ�����޵ȴ�
			{
				Temp_Node_NEXT=Temp_Node_LAST->NEXT;

				Temp_Node_LAST->NEXT=Add_Node;

				Add_Node->TimeOut_MS=TimeOut_MS;

				Add_Node->NEXT=Temp_Node_NEXT;
			}
		}
	}

	return Error_OK;
}
int Timer_Delete_Timer_Queue(int Handle,Timer_Node_Type **Delete_Node)
{
	if(Delete_Node==Null)
	{
		return Error_Invalid_Parameter;
	}
	Timer_Node_Type *Temp_Node=Null,*Temp_Node_LAST=Null,*Temp_Node_NEXT=Null;

	Temp_Node=Timer_DATA.Timer_Queue.Begin;

	while(Temp_Node!=Null)
	{
		if((*Delete_Node)==Null)
		{
			if(Temp_Node->Handle==Handle)
			{
				*Delete_Node=Temp_Node;
			}
			else
			{
				goto Loop_End;
			}
		}
		else
		{
			if(Temp_Node==(*Delete_Node))
			{
				;
			}
			else
			{
				goto Loop_End;
			}
		}

		Temp_Node_NEXT=Temp_Node->NEXT;

		//������������ͷ
		if(Temp_Node_LAST==Null || Temp_Node==Timer_DATA.Timer_Queue.Begin)
		{
			Timer_DATA.Timer_Queue.Begin=Timer_DATA.Timer_Queue.Begin->NEXT;

			if(Timer_DATA.Timer_Queue.Begin!=Null)//��һ������
			{
				if(Timer_DATA.Timer_Queue.Begin->TimeOut_MS>=0)
				{
					if(Temp_Node->TimeOut_MS>=0)
					{
						Timer_DATA.Timer_Queue.Begin->TimeOut_MS=Timer_DATA.Timer_Queue.Begin->TimeOut_MS+Temp_Node->TimeOut_MS;
					}
				}
			}
		}
		//������������β
		if(Temp_Node_NEXT==Null || Temp_Node==Timer_DATA.Timer_Queue.End)
		{
			Timer_DATA.Timer_Queue.End=Temp_Node_LAST;

			if(Timer_DATA.Timer_Queue.End!=Null)
			{
				Timer_DATA.Timer_Queue.End->NEXT=Null;
			}
		}
		//���ͷ��β�����ǿ� ��ô�������м�
		if(Temp_Node_LAST!=Null && Temp_Node_NEXT!=Null)
		{
			//�޲�ʱ�����
			if(Temp_Node->TimeOut_MS>=0 && Temp_Node_NEXT->TimeOut_MS>=0)
			{
				Temp_Node_NEXT->TimeOut_MS=Temp_Node_NEXT->TimeOut_MS+Temp_Node->TimeOut_MS;
			}
			Temp_Node_LAST->NEXT=Temp_Node_NEXT;
		}
		Temp_Node->NEXT=Null;

		return Error_OK;
Loop_End:
		Temp_Node_LAST=Temp_Node;
		Temp_Node=Temp_Node->NEXT;
	}

	if((*Delete_Node)==Null)
	{
		return Error_Invalid_Handle;
	}
	else
	{
		return Error_Invalid_Parameter;
	}

}


//��ʱ��ִ�в��� ����API������ API_User
void __Timer_Task(void)
{
	Timer_Function_Type Timer_Function;

	FIFO_Queue_Open(Timer_DATA.FIFO_Queue);
	Timer_Function.Timer_Function=Null;
	while(1)
	{
		if(FIFO_Queue_Wait(Timer_DATA.FIFO_Queue,&Timer_Function,sizeof(Timer_Function_Type),Null,-1)==Error_OK)
		{
			if(Timer_Function.Timer_Function!=Null)
			{
				Timer_Function.Timer_Function(Timer_Function.Args);
				Timer_Function.Timer_Function=Null;
			}
		}
		else
		{
			Scheduling_Sleep_Task(10);
		}
	}
}

void __Timer_SysTick_Entry(void)
{
	if(Timer_DATA.Enabled==Disable || Timer_DATA.Timer_Queue.Begin==Null)
	{
		return ;
	}
	if(Timer_DATA.Timer_Queue.Begin->TimeOut_MS<0)
	{
		return ;
	}
	else
	{
		if(Timer_DATA.Timer_Queue.Begin->TimeOut_MS>0)
		{
			Timer_DATA.Timer_Queue.Begin->TimeOut_MS--;
		}
	}
	Timer_Node_Type *Temp_Node=Null;
	while(Timer_DATA.Timer_Queue.Begin!=Null)
	{
		if(Timer_DATA.Timer_Queue.Begin->TimeOut_MS!=0)
		{
			return ;
		}
		Temp_Node=Timer_DATA.Timer_Queue.Begin;

		Timer_DATA.Timer_Queue.Begin=Timer_DATA.Timer_Queue.Begin->NEXT;
		if(Timer_DATA.Timer_Queue.Begin==Null)
		{
			Timer_DATA.Timer_Queue.End=Null;
		}
		Temp_Node->NEXT=Null;

		//TODO
		__Sys_FIFO_Queue_Set(Timer_DATA.FIFO_Queue,&Temp_Node->Timer_Function,sizeof(Timer_Function_Type));


		Temp_Node->Suspended_Time_MS=-1;

		if(Temp_Node->Now_Countdown_N_Time_Cycle>0)
		{
			Temp_Node->Now_Countdown_N_Time_Cycle--;
			if(Temp_Node->Now_Countdown_N_Time_Cycle==0)
			{


				//����������
				if(Timer_Add_Timer_Queue(Temp_Node,-1)!=Error_OK)
				{
					;
				}
			}
		}
		else
		{
			if(Timer_Add_Timer_Queue(Temp_Node,Temp_Node->Cycle_Time_MS)!=Error_OK)
			{
				;
			}
		}

	}


}

