/*
 * SysCall.S
 *
 *  Created on: 2019��4��8��
 *      Author: Master.HE
 */

	MODULE SysCall

	SECTION .text:CODE:REORDER:NOROOT(2)

#if ((__ARM_ARCH == 6) || (__ARM_ARCH == 7)) && (__ARM_ARCH_PROFILE == 'M')

	THUMB

#elif (__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'R')

	ARM

#elif (__ARM_ARCH == 7) && (__ARM_ARCH_PROFILE == 'A')


#else

#error "123"

#endif

	PUBLIC SysCall

SysCall

	push {r4, r5, r6, r7,r8,r12}

	mov r12, r0
	mov r0, r1
	mov r1, r2
	mov r2, r3
	add r8, sp, #(4 * 6)
	ldmia r8!, {r3, r4, r5, r6,r7}
	mov r8, r12
	swi 0x0

	pop {r4, r5, r6, r7,r8,r12}
	//ldr	r1, =0xfffff000
	//cmp	r0, r1
	//bcs	1f
	bx lr



	END
