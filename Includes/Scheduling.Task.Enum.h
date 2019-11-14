/*
 * Scheduling.Task.Enum.h
 *
 *  Created on: 2019��4��23��
 *      Author: Master.HE
 */

#ifndef SCHEDULING_TASK_ENUM_H_
#define SCHEDULING_TASK_ENUM_H_

typedef enum
{
	Task_State_Runing							=0,	//����

	Task_State_Ready,								//׼��

	Task_State_Pend_Event_Flag,

	Task_State_Pend_Event_Flag_Group,				//����

	Task_State_Pend_FIFO_Queue,

	Task_State_Pend_Message_Mailboxes,

	Task_State_Pend_Message_Queue,

	Task_State_Pend_Mutex,

	Task_State_Pend_Semaphore,

	Task_State_Pend_Semaphore_Group,

	Task_State_Suspended,							//����

	Task_State_Event_Group,							//�¼���

	//Task_State_Delete,								//ɾ��

	Task_State_End,

}Task_State_Type;

#endif /* SCHEDULING_TASK_ENUM_H_ */
