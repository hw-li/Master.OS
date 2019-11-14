/*
 * Error.h
 *
 *  Created on: 2019��4��15��
 *      Author: Master.HE
 */

#ifndef ERROR_H_
#define ERROR_H_

typedef enum
{
	Error_Exist							=-14,					//�Ѿ�����
	Error_OverFlow						=-13,					//���
	Error_Empty							=-12,					//��

	Error_Busy							=-11,					//æµ
	Error_No_Open						=-10,					//δ��
	Error_Illegal						=-9,					//���Ϸ���
	Error_Dissatisfy					=-8,					//�޷��õ�����
	Error_Time_Out						=-7,					//��ʱ
	Error_Operation_Failed				=-6,					//����ʧ��
	Error_Unknown						=-5,					//δ֪����
	Error_Allocation_Memory_Failed		=-4,					//�ڴ����ʧ��
	Error_Invalid_Parameter				=-3,					//��Ч����
	Error_Invalid_Handle				=-2,					//��Ч���
	Error_Undefined						=-1,					//δ����

	Error_OK							=0,						//OK



}Error_Type;

#endif /* ERROR_H_ */
