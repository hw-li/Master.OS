/*
 * Memory.c
 *
 *  Created on: 2019��4��12��
 *      Author: Master.HE
 */
//#include "__Sys.API.h"
#include "Error.h"
#include "Memory.Define.h"
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
			(uint8_t *)__section_begin("__Sys_HEAP"),
			__section_size("__Sys_HEAP")))!=Error_OK)
	{
		return Err;
	}
#endif

#pragma section="HEAP"
	if((Err=__Memory_Init(
			&__Usr_Memory_DATA,
			(uint8_t *)__section_begin("HEAP"),
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
int __Memory_Test_List(Memory_Node_List_Type *P_List_DATA)
{
	if(P_List_DATA==Null)
	{
		return Error_Invalid_Parameter;
	}

	uint32_t Sum=0;

	Memory_Node_Type *Temp_Node=P_List_DATA->Begin;

	while(Temp_Node!=Null)
	{
		Sum=Sum+Temp_Node->Size_Byte;
		Temp_Node=Temp_Node->NEXT;
	}
	if(Sum==P_List_DATA->Size_Byte)
	{
		return Error_OK;
	}
	else
	{
		return Error_Dissatisfy;
	}
}
int __Memory_Init(
		Memory_DATA_Type *P_Memory_DATA,
		uint8_t *HEAP,
		uint32_t Size)
{
	if(P_Memory_DATA==Null || HEAP==Null|| ((uint32_t)HEAP&0x03)!=0)
	{
		return Error_Invalid_Parameter;
	}
	P_Memory_DATA->HEAP.Begin=HEAP;
	P_Memory_DATA->HEAP.End=(uint8_t *)((uint32_t)HEAP+Size-1);
	P_Memory_DATA->HEAP.Size_Byte=Size;

	int Err=Error_OK;

	Memory_Node_Type *Temp_Node;

	Temp_Node=(Memory_Node_Type *)&P_Memory_DATA->HEAP.Begin[0];

	//����������
	BUILD_BUG_ON(Memory_Node_Head_Size!=8);

	P_Memory_DATA->Malloc.Begin=Null;
	P_Memory_DATA->Malloc.End=Null;
	P_Memory_DATA->Malloc.Size_Byte=0;

	P_Memory_DATA->Free.Size_Byte=Size-Memory_Node_Head_Size;



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

#ifdef __Memory_TEST__
	//Test
	if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
	{
		P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

		int i=1;
		while(i);
	}
	if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
	{
		P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

		int i=1;
		while(i);
	}
#endif


	Memory_Node_Type *Temp_Node,*Temp_Node_LAST=Null;

	Temp_Node=P_Memory_DATA->Free.Begin;

	//��ʼ����Free�����������Ĵ�С�����ݽڵ�
	while(Temp_Node!=Null)
	{
		//�������ݿ�ͷ�Ƿ���ȷ
		if(__Memory_Check_Node_Verify(Temp_Node)!=Error_OK)
		{
			P_Memory_DATA->Flag.Free_Head_Err=1;
			return Null;
		}


		uint32_t Begin_Address=(uint32_t)Temp_Node+Memory_Node_Head_Size;
		//uint32_t End_Adress=Begin_Address+Temp_Node->Size_Byte-1;


		//��������Ҫ�����Ŀ�껹����ٸ��ֽ�
		uint32_t Size_Align=0;

		if((Begin_Address&(Align-1))!=0)
		{
			Size_Align=Align-(Begin_Address&(Align-1));
		}

		uint32_t New_Size=Size+Size_Align;

		//�ҵ���С�������������ݿ�
		if(Temp_Node->Size_Byte>=New_Size)
		{
			//��ַ����-��������С�ڵ���һ��ͷ�Ĵ�С���򲻽��зָ��Ϊ�ָ�û������
			if(Size_Align<=Memory_Node_Head_Size)
			{

#ifdef __Memory_TEST__
				uint32_t old_Free_Size=P_Memory_DATA->Free.Size_Byte;
				if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

					int i=1;
					while(i);
				}
#endif

			//���ȷָ����Ҫ�����ݽڵ��ʣ��δʹ�õ����ݽڵ�
				//������һ���������ݿ�
				Memory_Node_Type *Temp_Node_NEXT=Null;
				//��ǰ�ڵ���Էָ����һ��ʣ��δʹ�õĽڵ�
				if(Temp_Node->Size_Byte>(New_Size+Memory_Node_Head_Size))
				{
					//�������һ��δʹ�ýڵ���׵�ַ
					Temp_Node_NEXT=(Memory_Node_Type *)(Begin_Address+New_Size);

					//
					if(__Memory_Calculate_Node_Verify(Temp_Node_NEXT,Temp_Node->Size_Byte-New_Size-Memory_Node_Head_Size,Temp_Node->NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}
					//������ǰ����ڵ�ĳ���
					if(__Memory_Calculate_Node_Verify(Temp_Node,New_Size,Null)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}

					//��Ϊ���һ���ڵ�ͷ���ȼ���
					P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-Memory_Node_Head_Size;
				}
				else//��ǰ���ָܷ����һ��δʹ�õĽڵ�
				{
					Temp_Node_NEXT=Temp_Node->NEXT;

					//�����������Ľڵ�
					if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte,Null)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}
				}
				//���������������򳤶�
				P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-Temp_Node->Size_Byte;

				//������ڵ��Free������ȥ��
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

#ifdef __Memory_TEST__
				if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

					int i=1;
					while(i)
					{
						old_Free_Size=old_Free_Size;
						New_Size=New_Size;
						Size=Size;
						Align=Align;
					}
				}
#endif
				//������Ľڵ���ӵ�Malloc������
				//
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

#ifdef __Memory_TEST__
				//Test
				if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

					int i=1;
					while(i);
				}
				if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

					int i=1;
					while(i);
				}
#endif
				//��������ڵ�������׵�ַ
				return (void *)((uint32_t)Temp_Node+Memory_Node_Head_Size+Size_Align);

			}
			else//��ַ���������ڵ���һ���ڵ�ͷ����ô���Գ����и��2�����ݿ�
			{
				//�����Ƿ��������и�
				if(Temp_Node->Size_Byte>(Memory_Node_Head_Size+New_Size))
				{

					Memory_Node_Type *Temp_Node_Split;

					//�������ַ����Ľڵ���׵�ַ
					Temp_Node_Split=(Memory_Node_Type *)(Begin_Address+Size_Align-Memory_Node_Head_Size);


					//�ָ��һ����������Ľڵ�
					if(__Memory_Calculate_Node_Verify(Temp_Node_Split,Temp_Node->Size_Byte-(Size_Align-Memory_Node_Head_Size)-Memory_Node_Head_Size,Temp_Node->NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}

					//������ǰ�����ָ�Ľڵ�
					if(__Memory_Calculate_Node_Verify(Temp_Node,Size_Align-Memory_Node_Head_Size,Temp_Node_Split)!=Error_OK)
					{
						P_Memory_DATA->Flag.Unknown_Err=1;
						return Null;
					}

					//��ȥ����ָ�����Ľڵ�ͷ
					P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte-Memory_Node_Head_Size;
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

#ifdef __Memory_TEST__
	//Test
	if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
	{
		P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

		int i=1;
		while(i);
	}
	if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
	{
		P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

		int i=1;
		while(i);
	}
#endif

	Memory_Node_Type *Temp_Node,*Temp_Node_LAST=Null,*Temp_Node_NEXT=Null,*Temp_Node_Free=Null;

	Temp_Node=P_Memory_DATA->Malloc.Begin;

	//��Malloc�������ҵ���Ҫ�ͷŵĽڵ�
	while(Temp_Node!=Null)
	{
		//�������ݿ�ͷ�Ƿ���ȷ
		if(__Memory_Check_Node_Verify(Temp_Node)!=Error_OK)
		{
			P_Memory_DATA->Flag.Free_Head_Err=1;

			return ;
		}

		uint32_t Begin_Address_Free=(uint32_t)Temp_Node+Memory_Node_Head_Size;
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
			End_Adress_Free=Begin_Address_Free+Memory_Node_Head_Size+Temp_Node_Free->Size_Byte;

			//��Free���ҵ����ʵ�λ�÷Ž�ȥ���ߺϲ���
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
							//
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte+Memory_Node_Head_Size;

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte+Temp_Node->Size_Byte+Memory_Node_Head_Size,Temp_Node->NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else//End_Adress_Free<(uint32_t)Temp_Node//���ܺϲ������뵽ͷ֮ǰ
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Temp_Node)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}

						P_Memory_DATA->Free.Begin=Temp_Node_Free;

#ifdef __Memory_TEST__
						//Test
						if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
						{
							P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

							int i=1;
							while(i);
						}
						if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
						{
							P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

							int i=1;
							while(i);
						}
#endif

						return ;
					}
				}
				//
				if(Temp_Node_NEXT!=Null)
				{
					//�������ݿ�ͷ�Ƿ���ȷ
					if(__Memory_Check_Node_Verify(Temp_Node_NEXT)!=Error_OK)
					{
						P_Memory_DATA->Flag.Free_Head_Err=1;

						return ;
					}

					uint32_t Begin_Address_LAST=(uint32_t)Temp_Node;
					uint32_t End_Adress_LAST=Begin_Address_LAST+Memory_Node_Head_Size+Temp_Node->Size_Byte;

					uint32_t Begin_Address_NEXT=(uint32_t)Temp_Node_NEXT;
					//uint32_t End_Adress_NEXT=Begin_Address_NEXT+Temp_Node_NEXT->Size_Byte+Memory_Node_Head_Size;

					//(LAST<Free || LAST==Free) && (Free<NEXT || Free==NEXT)
					if(End_Adress_LAST<=Begin_Address_Free && End_Adress_Free<=Begin_Address_NEXT)
					{
						//�ȿ�ǰ���
						//(LAST<Free || LAST==Free)
						if(End_Adress_LAST<Begin_Address_Free)//���ܺϲ��嵽����
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+Temp_Node_Free->Size_Byte;

							if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte,Temp_Node_Free)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else if(End_Adress_LAST==Begin_Address_Free)//���Ժϲ�
						{
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+(Memory_Node_Head_Size+Temp_Node_Free->Size_Byte);

							if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte+(Memory_Node_Head_Size+Temp_Node_Free->Size_Byte),Temp_Node->NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}
							//����ϲ���ô��ǰ����ڵ�����ͷŵĽڵ���
							Temp_Node_Free=Temp_Node;
						}
						else//�����ܴ���
						{
							P_Memory_DATA->Flag.Unknown_Err=1;
							return ;
						}

						//����
						//(Free<NEXT || Free==NEXT)
						if(End_Adress_Free<Begin_Address_NEXT)//���ܺϲ�
						{
							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte,Temp_Node_NEXT)!=Error_OK)
							{
								P_Memory_DATA->Flag.Unknown_Err=1;
								return ;
							}

						}
						else if(End_Adress_Free==Begin_Address_NEXT)//���Ժϲ�
						{
							//��ΪNEXT�Ľڵ�ռ��Ѿ��������ˣ����Ժϲ���ֻ�ܶ��һ��NEXT�Ľڵ�ͷ��С
							P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+(Memory_Node_Head_Size+0);

							if(__Memory_Calculate_Node_Verify(Temp_Node_Free,Temp_Node_Free->Size_Byte+(Memory_Node_Head_Size+Temp_Node_NEXT->Size_Byte),Temp_Node_NEXT->NEXT)!=Error_OK)
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
#ifdef __Memory_TEST__
						//Test
						if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
						{
							P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

							int i=1;
							while(i);
						}
						if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
						{
							P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

							int i=1;
							while(i);
						}
#endif
						return ;
					}


				}
				else if(Temp_Node_NEXT==Null)
				{
					//��ǰ�����һ���ڵ���

					//NEXT<Free || NEXT==Free
					uint32_t Begin_Address_LAST=(uint32_t)Temp_Node;
					//uint32_t End_Adress_LAST=Begin_Address_LAST+Temp_Node->Size_Byte+Memory_Node_Head_Size;

					if(Begin_Address_LAST<Begin_Address_Free)//���ܺϲ�
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
					else if(Begin_Address_LAST==Begin_Address_Free)//���Ժϲ�
					{
						P_Memory_DATA->Free.Size_Byte=P_Memory_DATA->Free.Size_Byte+(Memory_Node_Head_Size+Temp_Node_Free->Size_Byte);

						if(__Memory_Calculate_Node_Verify(Temp_Node,Temp_Node->Size_Byte+(Memory_Node_Head_Size+Temp_Node_Free->Size_Byte),Null)!=Error_OK)
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
#ifdef __Memory_TEST__
					//Test
					if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
					{
						P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

						int i=1;
						while(i);
					}
					if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
					{
						P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

						int i=1;
						while(i);
					}
#endif
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
#ifdef __Memory_TEST__
				//Test
				if(__Memory_Test_List(&P_Memory_DATA->Malloc)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Malloc_Size_Err=1;

					int i=1;
					while(i);
				}
				if(__Memory_Test_List(&P_Memory_DATA->Free)!=Error_OK)
				{
					P_Memory_DATA->Flag.TEST_Free_Size_Err=1;

					int i=1;
					while(i);
				}
#endif
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


