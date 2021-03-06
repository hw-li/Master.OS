/*
 * Error.h
 *
 *  Created on: 2019年4月15日
 *      Author: Master.HE
 */

#ifndef ERROR_H_
#define ERROR_H_

typedef enum
{
	Error_Exist							=-14,					//已经存在
	Error_OverFlow						=-13,					//溢出
	Error_Empty							=-12,					//空

	Error_Busy							=-11,					//忙碌
	Error_No_Open						=-10,					//未打开
	Error_Illegal						=-9,					//不合法的
	Error_Dissatisfy					=-8,					//无法得到满足
	Error_Time_Out						=-7,					//超时
	Error_Operation_Failed				=-6,					//操作失败
	Error_Unknown						=-5,					//未知错误
	Error_Allocation_Memory_Failed		=-4,					//内存分配失败
	Error_Invalid_Parameter				=-3,					//无效参数
	Error_Invalid_Handle				=-2,					//无效句柄
	Error_Undefined						=-1,					//未定义

	Error_OK							=0,						//OK



}Error_Type;

#endif /* ERROR_H_ */
