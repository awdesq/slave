#include "stc12c5a60s2.h"
#include "UseFlash.h"
#include <intrins.h>
#ifdef USEFLASH

//为安全需要清除标志位
void IAP_Idle(void)
{
	Set0(IAP_CONTR, 7);				//禁止IAP操作
	IAP_CMD = IAP_IDLE;				//清除命令
	
}

//清除一个扇区
void IAP_Erase_Sector(uint addr)
{
	IAP_CONTR = WAITTIME12M;		//设置CPU等待时间,这个决定了
	Set1(IAP_CONTR, 7);				//允许IAP工作
	IAP_CMD = IAP_SECTOR_ERASE;		//命令送入命令寄存器
	IAP_ADDRL = addr;			   	//16位地址送入地址寄存器
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;			   	//文档规定必须先送5A再送A5,然后开始工作
	IAP_TRIG = 0xA5;				//CPU在此之后会停止工作，直到IAP——CONTR等待的时间结束
	_nop_();						//按文档等待一个周期
	IAP_Idle();						//为安全需要清除标志位
}

//编程一个字节
void IAP_Program_Byte(uint addr, uchar dataByte)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);	
	IAP_CMD = IAP_PROGRAM;
	IAP_ADDRH = addr >> 8;
	IAP_ADDRL = addr;
	IAP_DATA = dataByte;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
}

//读取一个字节
uchar IAP_Read_Byte(uint addr)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);
	IAP_CMD = IAP_READ;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
	return IAP_DATA;		
}

#endif