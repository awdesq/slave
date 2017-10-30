#include "stc12c5a60s2.h"
#include "MyLib.h"
#include <intrins.h>

#ifdef	USE_UART_FOR_AUTO_DOWNLOAD
//以下程序用串口1来实现自动下载,自定义下载中波特率就调为14400bps,这是对于12M的晶振,其它晶振请重新计算


#ifdef USE_SOME_DELAY
//一些 delay 函数
void delayms(unsigned int n)
{
	unsigned int ms;
	while(n--)
		for(ms = 0; ms < Const1ms; ms++);
}

void delay1us(void)   //误差 0us
{
    _nop_();  //if Keil,require use intrins.h
}

void delay2us(void)   //误差 0us
{
    unsigned char a;
    for(a=3;a>0;a--);
}

void delay5us(void)   //误差 0us
{
    unsigned char a;
    for(a=12;a>0;a--);
}

void delay10us(void)   //误差 0us
{
    unsigned char a;
    for(a=27;a>0;a--);
}

void delay100us(void)   //误差 0us
{
    unsigned char a,b;
    for(b=66;b>0;b--)
        for(a=3;a>0;a--);
}

void delay200us(void)   //误差 0us
{
    unsigned char a,b;
    for(b=6;b>0;b--)
        for(a=98;a>0;a--);
}

void delay500us(void)   //误差 0us
{
    unsigned char a,b;
    for(b=111;b>0;b--)
        for(a=12;a>0;a--);
}

void delay1ms(void)   //误差 0us
{
    unsigned char a,b;
    for(b=222;b>0;b--)
        for(a=12;a>0;a--);
}
#endif

#ifdef	USE_DATA_FLASH
							
void IAP_Idle(void)
{
	Set0(IAP_CONTR, 7);				/* disable Read/Program/Erase Sector */
	IAP_CMD = IAP_IDLE;
}

void IAP_Erase_Sector(uint addr)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				/* Enable IAP*/
	IAP_CMD = IAP_SECTOR_ERASE;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
}

void IAP_Program_Byte(uint addr, uchar dataByte)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				/* Enable IAP*/
	IAP_CMD = IAP_PROGRAM;
	IAP_ADDRH = addr >> 8;
	IAP_ADDRL = addr;
	IAP_DATA = dataByte;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
}

uchar IAP_Read_Byte(uint addr)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				// Enable IAP
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

#ifdef	USE_COMMULICATION_FUN
sbit	StateIn = P3^2;							//通讯接收端端口,注意将端口设为只读P1M1.n = 1 P1M0.n = 0
sbit	StateOut = P1^5; 						//要注意将StateOut设置为强推挽 P1M1 = 0 P1M0 = 1
uchar	MsgBuffer[MsgBufferLen];				//定义一个总线上消息接收的缓冲区
uchar	*pStart, *pEnd;							//这是缓冲区的首地址和末地址
uchar	*pMsg, *pNewMsg;						//一个指针指向下一个要处理的消息，一个指针指向最新的消息，注意消息应当是　先入先出FIFO 
uchar	Msg;									//用于存储要发送的数据		
uchar	T1Count;								//用于记录进入定时器中断的次数,由外部中断负责清０
bit		SendOrGet;								//指示是发送还是接收数据，这个是给Time1Interrupt所用的 1 SendMsg 0 GetMsg

void Init_For_Communication(void)
{
	//初始化定时器
	AUXR = AUXR | 0x40;  				// T1, 1T Mode
    TMOD = 0x10 | (TMOD & 0x0F);		//16位定时器
    TH1 = COMMUNICATION_TH1;			//初值
    TL1 = COMMUNICATION_TL1;			//初值
    ET1 = 1;							//定时器1允许位

	//初始化相关地址
	pStart = pMsg = pNewMsg = &MsgBuffer;		//获得缓冲区的首地址,末地址,新消息存储指针,要处理的消息指针
	pEnd = &MsgBuffer + MsgBufferLen - 1;

	//释放总线
	StateOut = 1;								//输出1时,三极管不导通,总线为高

	//设置IO口模式
	Set0(P1M1, 5);								//设置StateOut为强推挽
	Set1(P1M0, 5);
	Set1(P3M1, 2);								//设置StateIn为只读
	Set0(P3M0, 2);
	
	PX0 = 1;					//将外部中断设为最高优先级,定时器中断比它低一级
	IT0 = 1; 					//下降沿触发中断
   	IE0 = 0;					//清标志位
	EX0 = 1;					//开外部中断0
}

uchar GetMsgCount()
{
	if(pNewMsg >= pMsg)
		return pNewMsg - pMsg;
	else
		return MsgBufferLen - (pMsg - pNewMsg);	
}

uchar GetMsg()
{
	//如果pMsg == pNewMsg 说明没有消息可取,返回NULLMsg
	if(pMsg == pNewMsg)
		return NULLMsg;

	if(pMsg == pEnd)		//如果这个消息位于缓冲区的尾部,则指针将移到首部
	{
		pMsg = pStart;
		return *pEnd;
	}
	else					//如果不是尾部,将消息加1就行了
	{
		pMsg++;
		return *(pMsg - 1);
	}
}

uchar ClearMsg()
{
	uchar ClearedMsgCount;
	ClearedMsgCount = GetMsgCount();
	pMsg = pNewMsg = pStart;
	return ClearedMsgCount;
}

uchar SendMsg(uchar myMsg)
{
	while(TR1  == 1);				//如果TR1 == 1,说明要么在发送数据,要么已在接收数据,等待其结束
   	delay100us();					//这里要不要延时,视实验而定,还是加了比较安全,这样应该有时间足够让总线恢复到高电平
	EX0 = 0;						//为不引起中断,将外部中断关闭再发消息
	T1Count = 0;					//用于Time1计中断次数
	SendOrGet = SendMsgFlag;		//说明在定时器中断中用发送数据的那一套程序
	Msg = myMsg;					//存储要发送的数据到公用变量
	StateOut = 0;					//拉低总线电平,引起接收消息外部中断,并开始按时间发送消息
	TR1 = 1;						//启动定时器
	return SUCCESS;		
}
/*
通讯协议:
总线结构:		Out被主机用10K电阻拉高,从机和主机都可拉低它.
发送的数据:		1bit前导 + 8bit数据,从高位发到低位,每位1ms
对映关系:		总线上被拉低,代表数据0,高电平代表1,1bit前导应使总线拉低1ms
pMsg和pNewMsg指向同一位置 表示没有消息
pMsg指向要处理的消息
pNewMsg指向将要存储新消息的位置

*/
void Timer1Interrupt(void) interrupt 3
{
    TH1 = COMMUNICATION_TH1;
    TL1 = COMMUNICATION_TL1;
	T1Count++;						//能过T1Count可以知道是第几次进入中断
	if(SendOrGet == GetMsgFlag)		//以下用于GetMsg,如果T时间发一位数据,则中断间隔为T/2,在3T/2 5T/2......17T/2进行采样
	{
	
		//>2说明过了第一个引导码
		//%2=1说明是奇数个T/2,恰好位于一位数据的正中间
		//=17个T/2时是最后一位数据
		if(((T1Count % 2) == 1) && (T1Count > 2) && (T1Count <= 17))	
		{
			*pNewMsg = (*pNewMsg & 0xFE) | StateIn;	//第0位置上StateIn一样的数据
			if(T1Count < 17)		//等于17时那一位不要左移,就放在第0位,左移会让最高位丢失
				*pNewMsg <<= 1;
		}
		else if(T1Count == 18)		//第18次中断来移动指针
		{
			if(*pNewMsg != NULLMsg)	//看收到的消息是不是NULLMsg,如果是,可以加上这一句不移动消息堆栈指针
			{
				if(pNewMsg == pEnd)
				{
					if(pMsg != pStart)
						pNewMsg = pStart;
				}
				else if((pNewMsg + 1) != pMsg)
				{
					pNewMsg++; 
				}
			} 
			IE0 = 0;//清中断标志位
			EX0 = 1;//重新打开int0中断
			TR1 = 0;//停止定时器
		}		
	}
	else							//以下用于SendMsg 						   
	{
		if(T1Count == 18)		//等于18时表示已经发送完成
		{
			StateOut = 1;		//释放总线
			IE0 = 0;			//清外部中断标志 													 
			EX0 = 1;			//开中断监视
			TR1 = 0;			//关定时器,结束发送
		}
		else if(T1Count % 2 == 0)	//逢2个 T/2就发送一位	
		{
			StateOut = Msg & 0x80;	
			Msg <<= 1;	
		}		
	}
}

void Int0_Interrupt(void) interrupt 0
{
	EX0 = 0;//关闭int0,等一下这个中断服务程序要交出控制权给定定时器在总线上读数据,所以在读数据期间要关闭外部中断
	SendOrGet = GetMsgFlag;//表示Time1用于接收数据
	T1Count = 0;//这个是给Time1用的,让Time1中断服务程序知道从int0交控制权给它读数据时是第几次进入Time1中断服务程序
	TR1 = 1;//开启定时器开始读
}
#endif