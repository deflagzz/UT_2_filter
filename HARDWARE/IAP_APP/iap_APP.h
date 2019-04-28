#ifndef __IAP_APP_H__
#define __IAP_APP_H__
#include "stm32f10x.h"
#include "can.h"

/*�û��Զ����豸CAN_ID���豸����*/
#define IAP_CAN_ID			17
#define IAP_DeviceType		5
/*�û��Զ����豸CAN_ID���豸����
IAP_DeviceType:
�豸���ͣ�00:δ����
		  01:����
		  02:�ŵ���-8
		  03:�ŵ���-16
		  04:�ر�
		  05:������
		  06:ң����
*/




#define IAP_Bootloat_SIZE 		0x2000	//8K

#define PC2MCU_Rend_device_info 0x7D
#define MCU2PC_Rend_device_info 0x7B
#define PC2MCU_IAP				0x7F
#define MCU2PC_IAP 				0x7E


extern u32 g_CpuID[3];
extern u32 g_Lock_Code;

//�ú��������main�����ͷ
void IAP_APP_Init(void);	
//�ú��������CAN�����ж���
void IAP_APP_CAN_ReStart(CanRxMsg temp_CAN_Msg);//APP����CAN�ж���,ˢ�̼���ת����
u8 IAP_Send_Device_ino(void);				//����λ�������豸��Ϣ


#endif







































