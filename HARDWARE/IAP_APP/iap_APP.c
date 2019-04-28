#include "iap_APP.h"

//获取芯片唯一ID
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

//can发送一组数据(固定格式:ID为0X12,标准帧,数据帧)	
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
static u8 IAP_Can_Send_Msg(u8 ID,u8* msg,u8 len)
{	
	u8 mbox;
	u16 i=0;
	CanTxMsg TxMessage;
	TxMessage.StdId = ID;			// 标准标识符  
	TxMessage.ExtId = ID;			// 设置扩展标示符 
	TxMessage.IDE=CAN_Id_Standard; 	// 标准帧
	TxMessage.RTR=CAN_RTR_Data;		// 数据帧
	TxMessage.DLC=len;				// 要发送的数据长度
	for(i=0;i<len;i++)
	TxMessage.Data[i]=msg[i];			          
	mbox= CAN_Transmit(CAN1, &TxMessage);   
	i=0; 
	while((CAN_TransmitStatus(CAN1, mbox)!=CAN_TxStatus_Ok)&&(i<0XFFF))
		i++;	//等待发送结束
	if(i>=0XFFF)
		return 1;
	return 0;	 
}

/*1.DeviceType
	设备类型：01:主板
			  02:磁导航-8 
			  03:磁导航-16
			  04:
			  05：
			  06:RFID 地标
			  07:超声波避障
2.RunMode
	运行模式：01:BootLoader 
			  02:APP*/
u8 Send_response(u8 Can_ID,u8 Byte1,u8 Byte6,u8 Byte7,u8 Byte8)	//发送响应
{
	u8 temp[8]={0};

	GetLockCode(&g_Lock_Code);			//获取芯片唯一ID
		
	temp[0] = Byte1;					//命令字
	temp[1] = g_Lock_Code>>24&0xff;		//唯一ID
	temp[2] = g_Lock_Code>>16&0xff;	
	temp[3] = g_Lock_Code>>8&0xff;
	temp[4] = g_Lock_Code&0xff;
	temp[5] = Byte6;					//CAN_ID	//刷固件模式:接收数据长度3个字节
	temp[6] = Byte7;					//设备类型
	temp[7] = Byte8;					//运行模式 01:BootLoader  02:APP

	return IAP_Can_Send_Msg(Can_ID,temp,8);
}

//APP程序CAN中断内,刷固件跳转程序
void IAP_APP_CAN_ReStart(CanRxMsg temp_CAN_Msg)
{
	if(temp_CAN_Msg.StdId == PC2MCU_Rend_device_info)	//PC读设备唯一ID
	{
		if(temp_CAN_Msg.Data[0]==1&&temp_CAN_Msg.Data[1]==0&&temp_CAN_Msg.Data[2]==0&&temp_CAN_Msg.Data[3]==0&&
		   temp_CAN_Msg.Data[4]==0&&temp_CAN_Msg.Data[5]==0&&temp_CAN_Msg.Data[6]==0&&temp_CAN_Msg.Data[7]==0)
		{	
			IAP_Send_Device_ino();					//向上位机发送设备信息
		}
	
	}
	if(temp_CAN_Msg.StdId == PC2MCU_IAP)
	{	
		GetLockCode(&g_Lock_Code);						//获取芯片唯一ID

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
	SCB->VTOR = FLASH_BASE | IAP_Bootloat_SIZE; //IAP_Bootloat_SIZE:0x2000=8K 字节
}

//向上位机发送设备信息
u8 IAP_Send_Device_ino(void)
{
	return Send_response(MCU2PC_Rend_device_info,0x81,IAP_CAN_ID,IAP_DeviceType,2);
}


