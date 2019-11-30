/*
 * Memory.c
 *
 *  Created on: 2019��4��12��
 *      Author: Master.HE
 */
//#include "__Sys.API.h"
#include "Error.h"

#include "Memory.h"

#ifdef __MPU__
Memory_DATA_Type __Sys_Memory_DATA;
#endif

Memory_DATA_Type __Usr_Memory_DATA;


int __Sys_Memory_Init(void)
{
	int Err;

#ifdef __MPU__
#pragma section="__Sys_HEAP"
	if((Err=__Memory_Init(
			&__Sys_Memory_DATA,
			__section_begin("__Sys_HEAP"),
			__section_end("__Sys_HEAP"),
			__section_size("__Sys_HEAP")))!=Error_OK)
	{
		return Err;
	}
#endif

#pragma section="HEAP"
	if((Err=__Memory_Init(
			&__Usr_Memory_DATA,
			__section_begin("HEAP"),
			__section_end("HEAP"),
			__section_size("HEAP")))!=Error_OK)
	{
		return Err;
	}
	return Error_OK;
}

uint32_t __Sys_Memory_Size_Malloc(void)
{
#ifdef __MPU__
	return __Memory_Size_Malloc(&__Sys_Memory_DATA);
#else
	return __Memory_Size_Malloc(&__Usr_Memory_DATA);
#endif
}

uint32_t __Sys_Memory_Size_Free(void)
{
#ifdef __MPU__
	return __Memory_Size_Free(&__Sys_Memory_DATA);
#else
	return __Memory_Size_Free(&__Usr_Memory_DATA);
#endif
}


void *__Sys_Memory_Malloc_Align(uint32_t Size,uint32_t Align)
{
#ifdef __MPU__
	return __Memory_Malloc(&__Sys_Memory_DATA,Size,Align);
#else
	return __Memory_Malloc(&__Usr_Memory_DATA,Size,Align);
#endif
}

void *__Sys_Memory_Malloc(uint32_t Size)
{
#ifdef __MPU__
	return __Memory_Malloc(&__Sys_Memory_DATA,Size,4);
#else
	return __Memory_Malloc(&__Usr_Memory_DATA,Size,4);
#endif
}

void __Sys_Memory_Free(void *ap)
{
#ifdef __MPU__
	__Memory_Free(&__Sys_Memory_DATA,ap);
#else
	__Memory_Free(&__Usr_Memory_DATA,ap);
#endif
}

uint32_t __Usr_Memory_Size_Malloc(void)
{
	return __Memory_Size_Malloc(&__Usr_Memory_DATA);
}

uint32_t __Usr_Memory_Size_Free(void)
{
	return __Memory_Size_Free(&__Usr_Memory_DATA);
}

void *__Usr_Memory_Malloc_Align(uint32_t Size,uint32_t Align)
{
	return __Memory_Malloc(&__Usr_Memory_DATA,Size,Align);
}

void *__Usr_Memory_Malloc(uint32_t Size)
{
	return __Memory_Malloc(&__Usr_Memory_DATA,Size,4);
}

void __Usr_Memory_Free(void *ap)
{
	__Memory_Free(&__Usr_Memory_DATA,ap);
}

//-----------------------------------------------------------------------------------------
int __Memory_Init(
		Memory_DATA_Type *P_Memory_DATA,
		uint32_t *HEAP_Begin,
		uint32_t *HEAP_End,
		uint32_t HEAP_Size)
{
	if(P_Memory_DATA==Null || HEAP_End==Null || HEAP_Size==0)
	{
		return Error_Invalid_Parameter;
	}
	P_Memory_DATA->HEAP.Begin=HEAP_Begin;
	P_Memory_DATA->HEAP.End=HEAP_End;
	P_Memory_DATA->HEAP.Size_Byte=HEAP_Size;

	int Err=Error_OK;

	Memory_Node_Type *Temp_Node;

	Temp_Node=(Memory_Node_Type *)&P_Memory_DATA->HEAP.Begin[0];

	//����������
	BUILD_BUG_ON(sizeof(Memory_Node_Type)!=8);

	P_Memory_DATA->Malloc.Begin=Null;
	P_Memory_DATA->Malloc.End=Null;
	P_Memory_DATA->Malloc.Size_Byte=0;

	P_Memory_DATA->Free.Size_Byte=HEAP_Size-sizeof(Memory_Node_Type);



	if((Err=__Memory_Calculate_Node_Verify(Temp_Node,P_Memory_DATA->Free.Size_Byte,Null))!=Error_OK)
	{
		return Err;
	}

	P_Memory_DATA->Free.Begin=Temp_Node;
	P_Memory_DATA->Free.End=Null;//Temp_Node;

	P_Memory_DATA->Flag.DATA=0;



	return Error_OK;

}
uint32_t __Memory_Size_Malloc(Memory_DATA_Type *P_Memory_DATA)
{
	if(P_Memory_DATA==Null)
	{
		return 0;
	}
	return P_Memory_DATA->Malloc.Size_Byte;
}
uint32_t __Memory_Size_Free(Memory_DATA_Type *P_Memory_DATA)
{
	if(P_Memory_DATA==Null)
	{
		return 0;
	}
	return P_Memory_DATA->Free.Size_Byte;
}
//Align������2�ı����Ҳ���С��4
void *__Memory_Malloc(Memory_DATA_Type *P_Memory_DATA,uint32_t Size,uint32_t Align)
{
	if(P_Memory_DATA==Null || Size==0 || Align==0 || (Align&0x3)!=0)
	{
		return Null;
	}

	//��������4�ֽڶ���
	if((Size&0x03)!=0)
	{
		Size=(Size&0xFFFFFFFC)+0x04;
	}


	Memory_Node_Type *Temp_Node,*Temp_Node_LAST=Null;

	Temp_Node=P_Memory_DATA->Free.Begin;

	while(Temp_Node!=Null)
	{
		//�������ݿ�ͷ�Ƿ���ȷ
		if(__Memory_Check_Node_Verify(Temp_Node)!=Error_OK)
		{
			P_Memory_DATA->Flag.Free_Head_Err=1;
			return Null;
		}

		uint32_t Begin_Address=(uint32_t)Temp_Node+sizeof(Memory_Node_Type);
		//uint32_t End_Adress=Begin_Address+Temp_Node->Size_Byte-1;


		//�����ֽڶ���
		uint32_t Size_Align=0;

		if((Begin_Address&(Align-1))!=0)
		{
			Size_Align=Align-(Begin_Address&(Align-1));
		}

		//�ҵ���С�������������ݿ�
		if(Temp_Node->Size_Byte>=(Size+Size_Align))
		{
			//��ַ�������
			if(Size_Align<=sizeof(Memory_Node_Type))
			{
				//������һ���������ݿ�
				Memory_Node_Type *Temp_Node_NEXT=Null;
				if(Temp_Node->Size_Byte>(Size+Size_Align+sizeof(Memory_Node_Type)))
				{
					Temp_Node_NEXT=(Memory_Node_Type *)(Begin_Address+Size_Align+Size);

					if(__Memory_Calculate_Node_Verify(Temp_Node_NEXT,Temp_Node->Size_Byte-Size-Size_Align-sizeof(Memory_Node_Type),Temp_Node->NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}
					P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-sizeof(Memory_Node_Type);
				}
				else
				{
					Temp_Node_NEXT=Temp_Node->NEXT;
				}
				P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-(Size+Size_Align);

				if(Temp_Node_LAST==Null || Temp_Node==P_Memory_DATA->Free.Begin)
				{
					P_Memory_DATA->Free.Begin=Temp_Node_NEXT;
				}
				else
				{
					if(__Memory_Calculate_Node_Verify(Temp_Node_LAST,Temp_Node_LAST->Size_Byte,Temp_Node_NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}

				}

				if(__Memory_Calculate_Node_Verify(Temp_Node,Size+Size_Align,Null)!=Error_OK)
				{
					P_Memory_DATA->Flag.Unknown_Err=1;
					return Null;
				}
				P_Memory_DATA->Malloc.Size_Byte=P_Memory_DATA->Malloc.Size_Byte+Temp_Node->Size_Byte;

				if(P_Memory_DATA->Malloc.End!=Null)
				{
					if(__Memory_Check_Node_Verify(P_Memory_DATA->Malloc.End)!=Error_OK)
					{
						P_Memory_DATA->Flag.Malloc_Head_Err=1;
						return Null;
					}

					if(__Memory_Calculate_Node_Verify(P_Memory_DATA->Malloc.End,P_Memory_DATA->Malloc.End->Size_Byte,Temp_Node)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}
					P_Memory_DATA->Malloc.End=Temp_Node;
				}
				else
				{
					P_Memory_DATA->Malloc.Begin=Temp_Node;
					P_Memory_DATA->Malloc.End=Temp_Node;
				}

				return (void *)((uint32_t)Temp_Node+sizeof(Memory_Node_Type)+Size_Align);

			}
			else//��ַ�����㣬����һ���и�
			{

				//�и����㳤���Ƿ�����
				if(Temp_Node->Size_Byte>(Size+Size_Align+sizeof(Memory_Node_Type)))
				{

					Memory_Node_Type *Temp_Node_Split;

					Temp_Node_Split=(Memory_Node_Type *)(Begin_Address+Size_Align-sizeof(Memory_Node_Type));


					//�ָ��һ����������Ľڵ�
					if(__Memory_Calculate_Node_Verify(Temp_Node_Split,Temp_Node->Size_Byte-(Size_Align-sizeof(Memory_Node_Type))-sizeof(Memory_Node_Type),Temp_Node->NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}

					//������ǰ�����ָ�Ľڵ�
					if(__Memory_Calculate_Node_Verify(Temp_Node,Size_Align-sizeof(Memory_Node_Type),Temp_Node_Split)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}


					P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-sizeof(Memory_Node_Type);
				}
				

			}



		}

		Temp_Node_LAST=Temp_Node;

		Temp_Node=Temp_Node->NEXT;

	}
	P_Memory_DATA->Flag.Null_Err=1;
	return Null;


}

void __Memory_Free(Memory_DATA_Type *P_Memory_DATA,void *ap)
{
	if(P_Memory_DATA==Null || ap==Null)
	{
		return ;
	}

	Memory_Node_Type *Temp_Node,*Temp_Node_LAST=Null,*Temp_Node_NEXT=Null,*Temp_Node_Free=Null;

	Temp_Node=P_Memory_DATA->Malloc.Begin;

	while(Temp_Node!=Null)
	{
		//�������ݿ�ͷ�Ƿ���ȷ
		if(__Memory_Check_Node_Verify(Temp_Node)!=Error_OK)
		{
			P_Memory_DATA->Flag.Free_Head_Err=1;

			return ;
		}

		uint32_t Begin_Address_Free=(uint32_t)Temp_Node+sizeof(Memory_Node_Type);
		uint32_t End_Adress_Free=Begin_Address_Free+Temp_Node->Size_Byte-1;

		//�ҵ������Ҫ�ͷŵĽڵ�
		if((Begin_Address_Free<=(uint32_t)ap) && (uint32_t)ap<=End_Adress_Free)
		{
			//�ȴ�Malloc�����н���ǰ��Ҫ�ͷŵĽڵ��޳�
			if(Temp_Node_LAST==Null || Temp_Node==P_Memory_DATA->Malloc.Begin)
			{
				P_Memory_DATA->Malloc.Begin=Temp_Node->NEXT;
			}
			else
			{
				if(__Memory_Calculate_Node_Verify(Temp_Node_LAST,Temp_Node_LAST->Size_Byte,Temp_Node->NEXT)!=Error_OK)
				{
					P_Memory_DATA->Flag.Unknown_Err=1;
					return ;
				}

				//Temp_Node_LAST->NEXT=Temp_Node->NEXT;
			}

			if(Temp_Node==P_Memory_DATA->Malloc.End || Temp_Node->NEXT==Null)
			{
				P_Memory_DATA->Malloc.End=Temp_Node_LAST;

			}
			P_Memory_DATA->Malloc.Size_Byte=P_Memory_DATA->Malloc.Size_Byte-Temp_Node->Size_Byte;

			//�����յĽڵ�ŵ�free��
			Temp_Node_Free=Temp_Node;

			Temp_Node=P_Memory_DATA->Free.Begin;


			Begin_Address_Free=(uint32_t)Temp_Node_Free;
			End_Adress_Free=Begin_Address_Free+Temp_Node_Free->Size_Byte+sizeof(Memory_Node_Type);

			while(Temp_Node!=Null)
			{

				//�������ݿ�ͷ�Ƿ���ȷ
				if(__Memory_Check_Node_Verify(Temp_Node)!=Error_OK)
				{
					P_Memory_DATA->Flag.Free_Head_Err=1;

					return ;
				}
				Temp_Node_NEXT=Temp_Node->NEXT;

				//�ҳ����ʵ���һ�ں���һ�ڣ�Ȼ�� ��һ�� �м� ��һ�ںϲ�
				//������һ��ΪLAST��FreeΪ�м䣬��һ��ΪNEXT
				//LASTΪ��ǰTemp_Node
				//��ô�����ּ���

				//Free<LAST || Free==LAST

				//(LAST<Free || LAST==Free) && (Free<NEXT || Free==NEXT)

				//NEXT<Free || NEXT==Free


				//
				if(Temp_Node==P_Memory_DATA->Free.Begin)
				{
					//Free<LAST || Free==LAST

					//�ͷŵĽڵ�ĵ�ַ�ڵ�ǰ�ڵ�֮ǰ
					if(End_Adress_Free<=(uint32_t)Temp_Node)
					{
						//�ͷŵĽڵ���Ժ͵�ǰ�ڵ�ϲ�
						if(End_Adress_Free==(uint32_t)Temp_Node)
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte+Temp_Node->Size_Byte+sizeof(Memory_Node_Type);

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte+Temp_Node->Size_Byte+sizeof(Memory_Node_Type),Temp_Node->NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else//End_Adress_Free<(uint32_t)Temp_Node
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Temp_Node)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}

						P_Memory_DATA->Free.Begin=Temp_Node_Free;

						return ;
					}
				}
				else if(Temp_Node_NEXT!=Null)
				{
					//�������ݿ�ͷ�Ƿ���ȷ
					if(__Memory_Check_Node_Verify(Temp_Node_NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Free_Head_Err=1;

						return ;
					}

					uint32_t Begin_Address_LAST=(uint32_t)Temp_Node;
					uint32_t End_Adress_LAST=Begin_Address_LAST+Temp_Node->Size_Byte+sizeof(Memory_Node_Type);

					uint32_t Begin_Address_NEXT=(uint32_t)Temp_Node_NEXT;
					//uint32_t End_Adress_NEXT=Begin_Address_NEXT+Temp_Node_NEXT->Size_Byte+sizeof(Memory_Node_Type);

					//(LAST<Free || LAST==Free) && (Free<NEXT || Free==NEXT)
					if(End_Adress_LAST<=Begin_Address_Free && End_Adress_Free<=Begin_Address_NEXT)
					{
						//(LAST<Free || LAST==Free)
						if(End_Adress_LAST<Begin_Address_Free)
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

							if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte,Temp_Node_Free)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else if(End_Adress_LAST==Begin_Address_Free)
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte+sizeof(Memory_Node_Type);

							if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte+Temp_Node_Free->Size_Byte+sizeof(Memory_Node_Type),Temp_Node->NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}
							Temp_Node_Free=Temp_Node;
						}
						else//�����ܴ���
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}

						//(Free<NEXT || Free==NEXT)
						if(End_Adress_Free<Begin_Address_NEXT)
						{
							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Temp_Node_NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else if(End_Adress_Free==Begin_Address_NEXT)
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_NEXT->Size_Byte+sizeof(Memory_Node_Type);

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte+Temp_Node_NEXT->Size_Byte+sizeof(Memory_Node_Type),Temp_Node_NEXT->NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}
						}
						else//�����ܴ���
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}

						return ;
					}


				}
				else if(Temp_Node_NEXT==Null)
				{
					//NEXT<Free || NEXT==Free
					uint32_t Begin_Address_LAST=(uint32_t)Temp_Node;
					//uint32_t End_Adress_LAST=Begin_Address_LAST+Temp_Node->Size_Byte+sizeof(Memory_Node_Type);

					if(Begin_Address_LAST<Begin_Address_Free)
					{
						P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

						if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte,Temp_Node_Free)!=Error_OK)
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}

						if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Null)!=Error_OK)
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}
					}
					else if(Begin_Address_LAST==Begin_Address_Free)
					{
						P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte+sizeof(Memory_Node_Type);

						if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte+Temp_Node_Free->Size_Byte+sizeof(Memory_Node_Type),Null)!=Error_OK)
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}
					}
					else//�����ܴ���
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return ;
					}
					return ;
				}

				Temp_Node=Temp_Node->NEXT;
			}
			//����һ��������� ��ǰFree������û���κ�����
			if(P_Memory_DATA->Free.Begin==Null)
			{
				P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

				if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Null)!=Error_OK)
				{
					P_Memory_DATA->Flag.Unknown_Err=1;
					return ;
				}
				P_Memory_DATA->Free.Begin=Temp_Node_Free;

				return ;
			}
			else//����
			{
				P_Memory_DATA->Flag.Unknown_Err=1;
				return ;
			}

		}


		Temp_Node_LAST=Temp_Node;

		Temp_Node=Temp_Node->NEXT;
	}

	//û���ҵ�����ǰ��Ҫ�ͷŵĿռ䲻���ڣ�
	P_Memory_DATA->Flag.Free_Err=1;
	return ;


}
int __Memory_Calculate_Node_Verify(Memory_Node_Type *P_Node_DATA,uint32_t Size_Byte,Memory_Node_Type *P_Node_NEXT)
{
	if(P_Node_DATA==Null || Size_Byte>0x3FFFFFFF)
	{
		return Error_Invalid_Parameter;
	}
	P_Node_DATA->Size_Byte=Size_Byte;
	P_Node_DATA->NEXT=P_Node_NEXT;

	//Size_Byte=Size_Byte&0x3FFFFFFF;

	uint32_t NEXT_DATA=(uint32_t)P_Node_NEXT;

	int Sum_Size=0;
	int Sum_NEXT=0;
	for(int i=0;i<32;i++)
	{
		Sum_Size=Sum_Size+((Size_Byte>>i)&0x01);
		Sum_NEXT=Sum_NEXT+((NEXT_DATA>>i)&0x01);
	}
	P_Node_DATA->Flag_Size=Sum_Size&0x01;
	P_Node_DATA->Flag_NEXT=Sum_NEXT&0x01;

	return Error_OK;
}
int __Memory_Check_Node_Verify(Memory_Node_Type *P_Node_DATA)
{
	if(P_Node_DATA==Null)
	{
		return Error_Invalid_Parameter;
	}
	uint32_t Size_Byte=P_Node_DATA->Size_Byte;
	uint32_t NEXT_DATA=(uint32_t)P_Node_DATA->NEXT;

	int Sum_Size=0;
	int Sum_NEXT=0;
	for(int i=0;i<32;i++)
	{
		Sum_Size=Sum_Size+((Size_Byte>>i)&0x01);
		Sum_NEXT=Sum_NEXT+((NEXT_DATA>>i)&0x01);
	}

	if(	P_Node_DATA->Flag_Size!=(Sum_Size&0x01)
	|| P_Node_DATA->Flag_NEXT!=(Sum_NEXT&0x01))
	{
		return Error_Illegal;
	}
	return Error_OK;
}


