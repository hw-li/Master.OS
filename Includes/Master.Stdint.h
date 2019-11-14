/*
 * Master.Stdint.h
 *
 *  Created on: 2019��4��8��
 *      Author: Master.HE
 */

#ifndef MASTER_STDINT_H_
#define MASTER_STDINT_H_

#include <stdint.h>


typedef enum
{
	false=0,
	true,

	bool_End,
}bool;

typedef enum	//ʹ��״̬
{
	Disable=0,		//�ر�
	Enable,			//����

	Enabled_End
}Enabled_Type;



#define Null ((void *)0)

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#endif /* MASTER_STDINT_H_ */
