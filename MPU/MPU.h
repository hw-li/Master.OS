/*
 * MPU.h
 *
 *  Created on: 2019��11��18��
 *      Author: Master.HE
 */

#ifndef MPU_H_
#define MPU_H_

#ifdef __MPU__

#include "Machine/Machine.Struct.h"

int MPU_Init(Machine_Desc_MPU_Type *P_MPU);

#endif

#endif /* MPU_H_ */
