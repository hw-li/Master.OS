/*
 * Memory.c
 *
 *  Created on: 2019��4��12��
 *      Author: Master.HE
 */
//#include "__Sys.API.h"


#include "Memory.Struct.h"

#include "Memory.h"


Memory_Type Memory_DATA;



void Memory_Init(void)
{
#pragma section="HEAP"
	Memory_DATA.HEAP_Begin=__section_begin("HEAP");
	Memory_DATA.HEAP_End=__section_end("HEAP");
	Memory_DATA.HEAP_Size=__section_size("HEAP");



	Memory_Node_Type *Temp_Node;

	Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[0];

	//����������
	BUILD_BUG_ON(sizeof(Memory_Node_Type)!=4);


	Temp_Node->Size=(Memory_DATA.HEAP_Size/4)-1;
	Temp_Node->Free=1;

	Temp_Node->NEXT_Index=(Memory_DATA.HEAP_Size/4);


	Memory_DATA.Size_Byte=Temp_Node->Size*4;

}
uint32_t __Sys_Memory_Size(void)
{
	return Memory_DATA.HEAP_Size;
}

void *__Sys_Memory_Malloc(unsigned int Size)
{
	Memory_Node_Type *Temp_Node,*Temp_Node1;

	void *Re;

	uint32_t Index;

	uint16_t sy=0;

	if(Size==0)
	{
		return Null;
	}

	if((Size%4)==0)
	{
		Size=Size/4;
	}
	else
	{
		Size=Size/4+1;
	}

	Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[0];

	while(1)
	{
		if(Temp_Node->Free==1 && Temp_Node->Size>=Size)
		{
			Temp_Node->Free=0;			//����� ռ��

			sy=Temp_Node->Size-Size;	//����ʣ��ռ�

			Temp_Node->Size=Size;		//���ռ�ÿռ��С

			Memory_DATA.Size_Byte=Memory_DATA.Size_Byte-Size*4;

			Re=(void*)((uint32_t)Temp_Node+4);

			if(sy>1)//ʣ��ռ�����2����λ
			{

				Index=(uint32_t)Temp_Node-(uint32_t)&Memory_DATA.HEAP_Begin[0];//���㵱ǰƫ����

				Index=Index/4;

				Temp_Node1=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[Index+Size+1];

				Temp_Node1->NEXT_Index=Temp_Node->NEXT_Index;

				Temp_Node->NEXT_Index=Index+Size+1;

				Temp_Node1->Free=1;
				Temp_Node1->Size=sy-1;

				Memory_DATA.Size_Byte=Memory_DATA.Size_Byte-4;
			}

			return Re;
		}
		if(Temp_Node->NEXT_Index==(Memory_DATA.HEAP_Size/4))
		{
			return Null;
		}
		else
		{
			Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[Temp_Node->NEXT_Index];
		}
	}
}

void __Sys_Memory_Free(void *ap)
{
	Memory_Node_Type *Temp_Node,*Temp_Node_LAST=Null,*Temp_Node_NEXT;

	if(ap==Null)
	{
		return;
	}

	Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[0];

	while(1)
	{
		if(Temp_Node->Free==0)
		{

			Temp_Node_NEXT=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[Temp_Node->NEXT_Index];

			if(((uint32_t)Temp_Node)<=((uint32_t)ap) && ((uint32_t)ap)<=((uint32_t)Temp_Node_NEXT))
			{
				Temp_Node->Free=1;

				Memory_DATA.Size_Byte=Memory_DATA.Size_Byte+Temp_Node->Size*4;


				//����֮ǰ��
				if(Temp_Node_LAST!=Null)//��һ���ڵ�����������ǿ��� ��ô�ϲ�
				{
					if(Temp_Node_LAST->Free==1)
					{
						Temp_Node_LAST->Size=Temp_Node_LAST->Size+Temp_Node->Size+1;

						Temp_Node_LAST->NEXT_Index=Temp_Node->NEXT_Index;

						Memory_DATA.Size_Byte=Memory_DATA.Size_Byte+4;

						Temp_Node=Temp_Node_LAST;
					}
					else
					{
						return;//�������������
					}

				}
				else
				{
					Temp_Node_LAST=Null;
				}

				//��������
				Temp_Node_LAST=Temp_Node;
				while(1)
				{
					if(Temp_Node->NEXT_Index==(Memory_DATA.HEAP_Size/4))
					{
						return;
					}
					else
					{
						Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[Temp_Node->NEXT_Index];
					}

					if(Temp_Node->Free==1)
					{
						if(Temp_Node_LAST!=Null)
						{
							if(Temp_Node_LAST->Free==1)
							{
								Temp_Node_LAST->Size=Temp_Node_LAST->Size+Temp_Node->Size+1;

								Temp_Node_LAST->NEXT_Index=Temp_Node->NEXT_Index;

								Memory_DATA.Size_Byte=Memory_DATA.Size_Byte+4;

								Temp_Node=Temp_Node_LAST;
							}
							else
							{
								return;//�������������
							}
						}
						else
						{
							return;//�������������
						}

					}
					else
					{
						return;
					}

				}

			}
			else
			{
				Temp_Node_LAST=Null;
			}

		}
		else //Temp_Node->Free==1
		{
			//����֮ǰ��
			if(Temp_Node_LAST!=Null)//��һ���ڵ�����������ǿ��� ��ô�ϲ�
			{
				if(Temp_Node_LAST->Free==1)
				{
					Temp_Node_LAST->Size=Temp_Node_LAST->Size+Temp_Node->Size+1;

					Temp_Node_LAST->NEXT_Index=Temp_Node->NEXT_Index;

					Memory_DATA.Size_Byte=Memory_DATA.Size_Byte+4;

					Temp_Node=Temp_Node_LAST;
				}
				else
				{
					return;//�������������
				}

			}
			else//Temp_Node_LAST==Null
			{
				Temp_Node_LAST=Temp_Node;
			}
		}
		if(Temp_Node->NEXT_Index==(Memory_DATA.HEAP_Size/4))
		{
			break;
		}
		else
		{
			Temp_Node=(Memory_Node_Type *)&Memory_DATA.HEAP_Begin[Temp_Node->NEXT_Index];
		}
	}

}
