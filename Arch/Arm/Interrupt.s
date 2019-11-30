/*
 * Interrupt.S
 *
 *  Created on: 2019��4��4��
 *      Author: Master.HE
 */
#include "Interrupt_Header.inc"

	MODULE Interrupt

#if ((__ARM_ARCH == 6) || (__ARM_ARCH == 7)) && (__ARM_ARCH_PROFILE == 'M')

	SECTION .text:CODE:REORDER:NOROOT(2)


	EXTERN __SysCall_Table
	EXTERN __IRQ_Entry
	//EXTERN __Scheduling_SysTick_Entry
	//EXTERN __Scheduling_PendSV_Entry
	//EXTERN __Timer_SysTick_Entry


	THUMB

	PUBLIC __Invalid_Entry
__Invalid_Entry
	B __Invalid_Entry

	PUBLIC __SVC_Entry
__SVC_Entry


	Exception_Entry


	sub sp,sp,#0x0c

	str r6,[sp,#0x8]
	str r5,[sp,#0x4]
	str r4,[sp]

	ldr r6,=__SysCall_Table
	lsls r7,r7,#2
	ldr r7,[r6,r7]
	blx r7

	add sp,sp,#0x0c

	MRS R7, PSP

	str r0,[r7]


	Exception_Slow_Exit

	bx lr

	//B __SVC_Entry
/*
	PUBLIC __PendSV_Entry
__PendSV_Entry
	Exception_Entry



	ldr r0,=__Scheduling_PendSV_Entry
	blx r0



	Exception_Slow_Exit

	bx lr
	//B __PendSV_Entry
*/

/*
	PUBLIC __SysTick_Entry
__SysTick_Entry

	Exception_Entry

	ldr r0,=__Scheduling_SysTick_Entry
	blx r0

	ldr r0,=__Timer_SysTick_Entry
	blx r0

	Exception_Fast_Exit

	bx lr
*/



	PUBLIC __Interrupt_Entry
__Interrupt_Entry


	Exception_Entry


	ldr r1,=__IRQ_Entry
	blx r1


	Exception_Fast_Exit

	bx lr


	//B __Interrupt_Entry

#elif (__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'R')

	SECTION .text:CODE:NOROOT(2)

	EXTERN __SysCall_Table
	EXTERN __IRQ_Entry

#ifdef __MPU__
	EXTERN __Sys_Scheduling_GET_System_SP_End
	EXTERN __Sys_Scheduling_GET_User_SP
	EXTERN MPU_Handle_Data_Fault
#endif

	ARM

	//��ȨģʽM[4��0]=0b10011-Supervisor
	PUBLIC __SVC_Entry
__SVC_Entry

	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ

	stmdb sp!,{r0-r12,lr}//ѹջ

#ifdef __MPU__

	mov r9,r0
	mov r10,r1
	mov r11,r2
	mov r12,r3

	ldr r0,=__Sys_Scheduling_GET_User_SP
	blx r0
	str sp,[r0]

	ldr r0,=__Sys_Scheduling_GET_System_SP_End
	blx r0

	msr cpsr_c, #(SVC_MODE|I_Bit|F_Bit)//�л���SVCģʽ

	ldr sp,[r0]

	mov r0,r9
	mov r1,r10
	mov r2,r11
	mov r3,r12

#else
	mov r12,sp

	msr cpsr_c, #(SVC_MODE|I_Bit|F_Bit)//�л���SVCģʽ

	mov sp,r12
#endif

	STMDB sp!, {lr}
//

	sub sp,sp,#0x0c

	str r6,[sp,#0x8]
	str r5,[sp,#0x4]
	str r4,[sp]

	ldr r6,=__SysCall_Table
	lsls r7,r7,#2
	ldr r7,[r6,r7]
	blx r7

	add sp,sp,#0x0c


//


#ifdef __MPU__

	mov r12,r0

	ldr r0,=__Sys_Scheduling_GET_User_SP
	blx r0

	LDMIA sp!, {lr}

	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ

	ldr sp,[r0]

	mov r0,r12

#else

	LDMIA sp!, {lr}

	mov r12,sp
	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ

	mov sp,r12
#endif

	str r0,[sp]//��������ֵ

	ldmia sp!,{r0-r12,lr}

	msr cpsr_c, #(SVC_MODE|I_Bit|F_Bit)//�л���SVCģʽ

	mov sp,#0//����ں˶�ջָ��

	MOVS PC,LR

	//b __SVC_Entry

//---------------------------------
	//�ⲿ�ж�����M[4��0]=0b10010-IRQ
	PUBLIC __Interrupt_Entry
__Interrupt_Entry

	SUB LR, LR, #4

	msr cpsr_c, #(SYS_MODE | I_Bit | F_Bit)//�л���sysģʽ

	stmdb sp!,{r0-r12,lr}//ѹջ

#ifdef __MPU__

	ldr r0,=__Sys_Scheduling_GET_User_SP
	blx r0
	str sp,[r0]

	ldr r0,=__Sys_Scheduling_GET_System_SP_End
	blx r0

	msr cpsr_c, #(IRQ_MODE|I_Bit|F_Bit)//�л���IRQģʽ

	ldr sp,[r0]

#else

	mov r12,sp

	msr cpsr_c, #(IRQ_MODE|I_Bit|F_Bit)//�л���IRQģʽ

	mov sp,r12

#endif

	STMDB sp!, {lr}
//


	ldr r1,=__IRQ_Entry
	blx r1

//

#ifdef __MPU__

	ldr r0,=__Sys_Scheduling_GET_User_SP
	blx r0

	LDMIA sp!, {lr}

	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ

	ldr sp,[r0]

#else

	LDMIA sp!, {lr}

	mov r12,sp
	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ

	mov sp,r12

#endif

	ldmia sp!,{r0-r12,lr}

	msr cpsr_c, #(IRQ_MODE|I_Bit|F_Bit)//�л���IRQģʽ
	mov sp,#0//����ں˶�ջָ��
	MOVS PC,LR

	//b __Interrupt_Entry
//---------------------------------------------------------------------
	//δ����ָ����ֹ//M[4��0]=0b11011-Undefined
	PUBLIC Undefined_Handler
Undefined_Handler


	b Undefined_Handler
//---------------------------------------------------------------------
	//Ԥȡָ�������ֹ//M[4��0]=0b10111-Abort
	PUBLIC Prefetch_Handler
Prefetch_Handler

	SUB LR, LR, #4


	b Prefetch_Handler
//---------------------------------------------------------------------
	//���ݷ�����ֹ//M[4��0]=0b10111-Abort
	PUBLIC Abort_Handler
Abort_Handler

	SUB LR, LR, #8

	stmdb sp!,{r0-r12,lr}//ѹջ

#ifdef __MPU__

	MRS r0, spsr
	and r0 ,r0, #MODE_MSK


	cmp r0,#USR_MODE

	BEQ Abort_Handler_GET_USR_SYS_SP

	cmp r0,#SYS_MODE

	BEQ Abort_Handler_GET_USR_SYS_SP

	cmp r0,#FIQ_MODE

	BEQ Abort_Handler_GET_FIQ_SP

	cmp r0,#IRQ_MODE

	BEQ Abort_Handler_GET_IRQ_SP

	cmp r0,#SVC_MODE

	BEQ Abort_Handler_GET_SVC_SP

Abort_Handler_GET_USR_SYS_SP

	msr cpsr_c, #(SYS_MODE|I_Bit|F_Bit)//�л���sysģʽ
	mov r1,sp
	b Abort_Handler_GET_SP_Exit

Abort_Handler_GET_FIQ_SP
	msr cpsr_c, #(FIQ_MODE|I_Bit|F_Bit)//�л���FIQģʽ
	mov r1,sp
	b Abort_Handler_GET_SP_Exit

Abort_Handler_GET_IRQ_SP
	msr cpsr_c, #(IRQ_MODE|I_Bit|F_Bit)//�л���IRQģʽ
	mov r1,sp
	b Abort_Handler_GET_SP_Exit

Abort_Handler_GET_SVC_SP
	msr cpsr_c, #(SVC_MODE|I_Bit|F_Bit)//�л���SVCģʽ
	mov r1,sp
	b Abort_Handler_GET_SP_Exit


Abort_Handler_GET_SP_Exit

	msr cpsr_c, #(ABT_MODE|I_Bit|F_Bit)//�л���ABTģʽ

	mov r2, lr


	ldr r4,=MPU_Handle_Data_Fault
	blx r4


#else
Abort_Handler_Undefined

	b Abort_Handler_Undefined

#endif

	ldmia sp!,{r0-r12,pc}^

	b Abort_Handler
//---------------------------------------------------------------------
#elif (__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'A')


#else

#error "123"

#endif


	END
