/*
 * Scheduling.h
 *
 *  Created on: 2019��4��23��
 *      Author: Master.HE
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#include "Scheduling.Task.Enum.h"
#include "Scheduling.Task.Define.h"

#include "__Sys.Scheduling.Task.Struct.h"

int Scheduling_Init(void);

int __Sys_Scheduling_Create_Task(
		char *Name,
		Task_Enter_Function Task_Enter,
		void *Args,
		Task_Exit_Function Task_Exit,
		uint8_t Priority,
		uint32_t *Stack,
		uint32_t Stack_Size_4Byte,
		int Option);

int __Sys_Scheduling_Release_Task(int Handle);

int __Sys_Scheduling_Sleep_Task(int32_t TimeOut);

int __Sys_Scheduling_Suspend_Task(int Handle);

int __Sys_Scheduling_Resume_Task(int Handle);

int Scheduling_Create_Task_Idle(
		char *Name,
		uint8_t Priority);

void Scheduling_Switch_To_Idle(void);

void Scheduling_SysTick(void);


void __Sys_Scheduling_Try_Context_Switch(void);

int __Sys_Scheduling_Context_Switch(Task_State_Type CS_Task_State,int32_t TimeOut,int32_t *RTimeOut);

int __Sys_Scheduling_GET_Current_TCB(__Sys_Scheduling_Task_TCB_Type **Current_TCB);

#ifdef __MPU__
uint32_t **__Sys_Scheduling_GET_System_SP_End(void);
uint32_t **__Sys_Scheduling_GET_User_SP(void);
void __Sys_Scheduling_MPU_SET_Current_Task(void);
void __Sys_Scheduling_MPU_SET_LAST_Task(void);
int __Sys_Scheduling_MPU_Add_Stack(uint32_t Mode,uint32_t *Fault_SP,uint32_t Fault_Address);
int __Sys_Scheduling_MPU_Add_Stack_Malloc(bool Sys_Memory,Scheduling_Task_TCB_Stack_Type *P_TCB_Stack,uint32_t Mode,uint32_t *Fault_SP,uint32_t Fault_Address);
#endif

#endif /* SCHEDULING_H_ */
