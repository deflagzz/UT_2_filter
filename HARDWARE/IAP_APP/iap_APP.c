#include "iap_APP.h"

//��ȡоƬΨһID
u32 g_CpuID[3];
u32 g_Lock_Code;	



void systeam_ReStart(void)
{
	u32 time = 65535;
	while(time--)
	{
		__NOP();
	}
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}


void GetLockCode(u32 *lock_ID)
{
	//??CPU??ID
	g_CpuID[0]=*(vu32*)(0x1ffff7e8);
	g_CpuID[1]=*(vu32*)(0x1ffff7ec);
	g_CpuID[2]=*(vu32*)(0x1ffff7f0);
	//????,????????
	*lock_ID=((g_CpuID[0]>>0)+(g_CpuID[1]>>1)+(g_CpuID[2]>>2)) / 2;
}

//can����һ������(�̶���ʽ:IDΪ0X12,��׼֡,����֡)	
//len:���ݳ���(���Ϊ8)				     
//msg:����ָ��,���Ϊ8���ֽ�.
//����ֵ:0,�ɹ�;
//		 ����,ʧ��;
static u8 IAP_Can_Send_Msg(u8 ID,u8* msg,u8 len)
{	
	u8 mbox;
	u16 i=0;
	CanTxMsg TxMessage;
	TxMessage.StdId = ID;			// ��׼��ʶ��  
	TxMessage.ExtId = ID;			// ������չ��ʾ�� 
	TxMessage.IDE=CAN_Id_Standard; 	// ��׼֡
	TxMessage.RTR=CAN_RTR_Data;		// ����֡
	TxMessage.DLC=len;				// Ҫ���͵����ݳ���
	for(i=0;i<len;i++)
	TxMessage.Data[i]=msg[i];			          
	mbox= CAN_Transmit(CAN1, &TxMessage);   
	i=0; 
	while((CAN_TransmitStatus(CAN1, mbox)!=CAN_TxStatus_Ok)&&(i<0XFFF))
		i++;	//�ȴ����ͽ���
	if(i>=0XFFF)
		return 1;
	return 0;	 
}

/*1.DeviceType
	�豸���ͣ�01:����
			  02:�ŵ���-8 
			  03:�ŵ���-16
			  04:
			  05��
			  06:RFID �ر�
			  07:����������
2.RunMode
	����ģʽ��01:BootLoader 
			  02:APP*/
u8 Send_response(u8 Can_ID,u8 Byte1,u8 Byte6,u8 Byte7,u8 Byte8)	//������Ӧ
{
	u8 temp[8]={0};

	GetLockCode(&g_Lock_Code);			//��ȡоƬΨһID
		
	temp[0] = Byte1;					//������
	temp[1] = g_Lock_Code>>24&0xff;		//ΨһID
	temp[2] = g_Lock_Code>>16&0xff;	
	temp[3] = g_Lock_Code>>8&0xff;
	temp[4] = g_Lock_Code&0xff;
	temp[5] = Byte6;					//CAN_ID	//ˢ�̼�ģʽ:�������ݳ���3���ֽ�
	temp[6] = Byte7;					//�豸����
	temp[7] = Byte8;					//����ģʽ 01:BootLoader  02:APP

	return IAP_Can_Send_Msg(Can_ID,temp,8);
}

//APP����CAN�ж���,ˢ�̼���ת����
void IAP_APP_CAN_ReStart(CanRxMsg temp_CAN_Msg)
{
	if(temp_CAN_Msg.StdId == PC2MCU_Rend_device_info)	//PC���豸ΨһID
	{
		if(temp_CAN_Msg.Data[0]==1&&temp_CAN_Msg.Data[1]==0&&temp_CAN_Msg.Data[2]==0&&temp_CAN_Msg.Data[3]==0&&
		   temp_CAN_Msg.Data[4]==0&&temp_CAN_Msg.Data[5]==0&&temp_CAN_Msg.Data[6]==0&&temp_CAN_Msg.Data[7]==0)
		{	
			IAP_Send_Device_ino();					//����λ�������豸��Ϣ
		}
	
	}
	if(temp_CAN_Msg.StdId == PC2MCU_IAP)
	{	
		GetLockCode(&g_Lock_Code);						//��ȡоƬΨһID

		if(	temp_CAN_Msg.Data[0]==0x01&&	
			temp_CAN_Msg.Data[1]==(g_Lock_Code>>24&0xff)&& 
			temp_CAN_Msg.Data[2]==(g_Lock_Code>>16&0xff)&&
			temp_CAN_Msg.Data[3]==(g_Lock_Code>>8&0xff)&&	
			temp_CAN_Msg.Data[4]==(g_Lock_Code&0xff) )
		{	
			systeam_ReStart();	
		}			
		
	}
}

void IAP_APP_Init(void)
{
	SCB->VTOR = FLASH_BASE | IAP_Bootloat_SIZE; //IAP_Bootloat_SIZE:0x2000=8K �ֽ�
}

//����λ�������豸��Ϣ
u8 IAP_Send_Device_ino(void)
{
	return Send_response(MCU2PC_Rend_device_info,0x81,IAP_CAN_ID,IAP_DeviceType,2);
}


