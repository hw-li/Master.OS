/*
 * Event.Enum.h
 *
 *  Created on: 2019��4��29��
 *      Author: Master.HE
 */

#ifndef EVENT_ENUM_H_
#define EVENT_ENUM_H_



typedef enum
{
	Event_Queue_Option_FIFO=0,
	Event_Queue_Option_Priority,

	Event_Queue_Option_End,
}Event_Queue_Option_Type;

typedef enum
{
	Event_Time_Out_Query				=-2,		//��ѯ

	Event_Time_Out_Infinite				=-1,		//���޵ȴ�

	Event_Time_Out_Occupy_Return_Back	=0,			//����ռ�ò���������

}Event_Time_Out_Type;



typedef enum
{
	Event_Flag_Group_Clear_Any_Read_Retain		=0,					//�κζ�ȡ������
	Event_Flag_Group_Clear_Any_Read_Clear,							//�κζ�ȡ�����


	//Event_Flag_Group_Clear_Any_Read_Result_OK_Retain_NoOK_Retain,	//���OK���� NoOK����

	Event_Flag_Group_Clear_Any_Read_Result_OK_Retain_Dissatisfy_Clear,//���OK���� Dissatisfy���

	Event_Flag_Group_Clear_Any_Read_Result_OK_Clear_Dissatisfy_Retain,//���OK��� Dissatisfy����

	//Event_Flag_Group_Clear_Any_Read_Result_OK_Clear_NoOK_Clear,		//���OK��� NoOK���

	Event_Flag_Group_Clear_End,
}Event_Flag_Group_Clear_Type;

#endif /* EVENT_ENUM_H_ */
