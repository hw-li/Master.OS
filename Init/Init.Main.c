/*
 * Init.Main.c
 *
 *  Created on: 2019��9��17��
 *      Author: Master.HE
 */
#include "Error.h"
#include "Define.h"

#include "Memory/Memory.h"
#include "IRQ/IRQ.h"
#include "Event/Event.h"
#include "Device/Device.h"
#include "Timer/Timer.h"
#include "Queue/Queue.h"
#include "Scheduling/Scheduling.h"
#include "Shell/Shell.h"
#include "MPU/MPU.h"

#include "Init.Machine.h"
#include "Init.Main.h"
#include "Init.Module.h"

#include "Task/Task.h"

#include "__Sys.API.h"




Machine_Desc_Type *Machine_Desc=Null;

void Start_Kernel(void)
{
	

	//��ʼ��BSP
	if(Machine_Init(&Machine_Desc)!=Error_OK)
	{
		while(1);
	}

	//MPU
#ifdef __MPU__
	if(MPU_Init(&Machine_Desc->MPU)!=Error_OK)
	{
		while(1);
	}
#endif

	//�رտ��Ź�
	if(Machine_Wdog_Disable(&Machine_Desc->Wdog)!=Error_OK)
	{
		while(1);
	}

	//��ʼ��оƬ
	if(Machine_Init_CPU(&Machine_Desc->CPU)!=Error_OK)
	{
		while(1);
	}

	//��̬�ڴ��ʼ��
	if(__Sys_Memory_Init()!=Error_OK)
	{
		while(1);
	}

	//��ʼ�������豸����
	if(Device_Init()!=Error_OK)
	{
		while(1);
	}


	//��ʼ���ں��¼�����
	if(Event_Init()!=Error_OK)
	{
		while(1);
	}


	//��ʼ��IRQ
	if(IRQ_Init(&Machine_Desc->IRQ)!=Error_OK)
	{
		while(1);
	}

	//�ر������ж�
	if(__Sys_IRQ_All_Disable()!=Error_OK)
	{
		while(1);
	}

	//��ʼ����ʱ��
	if(Timer_Init(&Machine_Desc->Timer)!=Error_OK)
	{
		while(1);
	}

	//��ʼ���������
	if(Queue_Init()!=Error_OK)
	{
		while(1);
	}

	//��ʼ��������
	if(Scheduling_Init()!=Error_OK)
	{
		while(1);
	}

	//��ʼ��Shell
	if(Shell_Init(&Machine_Desc->UART)!=Error_OK)
	{
		;
	}

	//����һ����������
	if(Scheduling_Create_Task_Idle("Idle",Priority_Task_Idle)<Error_OK)
	{
		while(1);
	}

	//��������
	if(Module_Init_Task()!=Error_OK)
	{
		while(1);
	}

	//��ʼ������ģ��
	if(Module_Init_Sys_Device()!=Error_OK)
	{
		while(1);
	}

	//
	if(Module_Init_Sys_Com()!=Error_OK)
	{
		while(1);
	}

	//�������ж�
	if(__Sys_IRQ_All_Enable()!=Error_OK)
	{
		while(1);
	}

	//��ת����������
#pragma section="CSTACK"
	__Sys_Switch_To_Idle(__section_end("CSTACK"),__section_begin("CSTACK"),Task_Idle);

	//
	while(1)
	{


	}
}
