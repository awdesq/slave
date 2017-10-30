#include "stc12c5a60s2.h"
#include "UseCommunication.h"
#include "Delay.h"
#include "define.h"

#ifdef	USECOMMULICATION

/*
20131119 原来State 改为了 StateOut,不知道原来为什么会有之样的语法问题没有解决,这是未出硬件之前,先写好的程序,未经实验,所以有以上错误
本程序只适应于发送和接收都用P3.2的程序,也就是说不适应于现在的主机
20140928，刷程序的时候一定要注意，要启用外部晶振，因为使用内部晶振导致调试一下午没调试好
20140928  只用P3.2外部中断来实现单工双向通讯
20171014发现拉低电平时，强推挽不是最佳选择，而应该用开漏输出
*/
uchar	MsgBuffer[MsgBufferLen];				//定义一个总线上消息接收的缓冲区
uchar	*pStart, *pEnd;							//这是缓冲区的首地址和末地址
uchar	*pMsg, *pNewMsg;						//pMsg指向下一个要处理的消息，pNewMsg指针指向最新的消息，注意消息应当是　先入先出FIFO 
uchar	Msg;									//用于存储要发送的数据		
uchar	T1Count;								//用于记录进入定时器中断的次数,由外部中断负责清０
bit		SendOrGet;								//指示是发送还是接收数据，这个是给Time1Interrupt所用的 1 SendMsg 0 GetMsg
#ifdef MAIN_USE
bit		GlobalSingal;
#endif
void InitForCommunication(void)
{
	//初始化定时器
	AUXR |= 0x40;						//定时器时钟1T模式
	TMOD &= 0x0F;						//设置定时器模式
	TMOD |= 0x10;						//设置定时器模式
    TH1 = COMMUNICATION_TH1;			//初值，设置见UseCommunication中
    TL1 = COMMUNICATION_TL1;			//初值，设置见UseCommunication中
    ET1 = 1;							//定时器1允许位

	//初始化相关地址
	pStart = pMsg = pNewMsg = &MsgBuffer;		//获得缓冲区的首地址,末地址,新消息存储指针,要处理的消息指针
	pEnd = &MsgBuffer + MsgBufferLen - 1;
	
	//初始化中断设置
	PX0 = 1;									//将外部中断设为最高优先级,定时器中断比它低一级
	IT0 = 1; 									//下降沿触发中断
   	IE0 = 0;									//清标志位

	//释放总线 设置IO口模式 开中断
	EnterGetMsgState();
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
	//20141002不准备设置NULLMsg了，因为0xFF会发送不出去
	//只要在GetMsg之前，加入GetMsgCount()，而不直接用GetMsg()，取消息，不用返回值判断是否有消息，就不会出错
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
	//查看是否冲突
	while(TR1  == 1);				//如果TR1 == 1,说明要么在发送数据,要么已在接收数据,等待其结束
	
	//等待接收机完成保存工作，打开中断，准备下一次接收
   	delay100us();					//这里要不要延时,视实验而定,还是加了比较安全,这样应该有时间足够让总线恢复到高电平

	//初始化计数和中断程序中要执行的功能
	T1Count = 0;					//用于Time1计中断次数
	SendOrGet = SendMsgFlag;		//说明在定时器中断中用发送数据的那一套程序

	//得到要发送的消息，存储到公用变量中
	Msg = myMsg;					//存储要发送的数据到公用变量

	//关中断，避免重复触发
	EX0 = 0;						//关闭外部中断,避免在发消息过程中,使自己产生了外部中断
	IE0 = 0;						//清除外部中断请求标志位

	#ifdef USE_P15_FOR_SENDMSG
	Set0(P1M1, 5);					//设置StateOut为强推挽
	Set1(P1M0, 5);					//发送结束要设置为只读
	#else
	//更改中断模式
	//20171014发现拉低电平时，强推挽不是最佳选择，而应该用开漏输出
	Set1(P3M1, 2);					//将P3.2设置成开漏输出,这样才可以拉低总线
	Set1(P3M0, 2);

	//Set0(P3M1, 2);					//将P3.2设置成强推挽输出,这样才可以拉低总线
	//Set1(P3M0, 2);
	
	#endif

   	//开始发送
	#ifdef USE_P15_FOR_SENDMSG
	StateOut = 1;					//拉低总线
	#else 
	State = 0;	  					//拉低总线
	#endif

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
    TH1 = COMMUNICATION_TH1;		//为TH1 TL1赋值，以便下次以相同的时间产生中断，来实现发送消息和接收消息，
    TL1 = COMMUNICATION_TL1;		//本程序发送和接收采用的是同步通讯
	T1Count++;						//能过T1Count可以知道是第几次进入中断
	if(SendOrGet == GetMsgFlag)		//以下用于GetMsg,如果T时间发一位数据,则中断间隔为T/2,在3T/2 5T/2......17T/2进行采样
	{
	
		//>2说明过了第一个引导码
		//%2=1说明是奇数个T/2,恰好位于一位数据的正中间
		//=17个T/2时是最后一位数据
		//当第17次中断进入这里，表示最后一位正在取样，如果每发一位的时间为200us，则，离发送完数据还有100us
		if(((T1Count % 2) == 1) && (T1Count > 2) && (T1Count <= 17))	
		{
			*pNewMsg = (*pNewMsg & 0xFE) | State;	//第0位置上StateIn一样的数据
			if(T1Count < 17)		//等于17时那一位不要左移,就放在第0位,左移会让最高位丢失
				*pNewMsg <<= 1;
		}
		//当进入第18次中断，如果同步完全准时的话，表示离发送消息的设备发送完已经过了100us了，但此时接收的设备还未打开中断
		//准备好一下次接收消息，所以在发送消息的设备在发送完后，最好等待200us再发送下一个数据，这样接收设备才能够做好保存
		//消息，并打开中断准备好下一次接收
		else if(T1Count == 18)		//第18次中断来移动指针
		{
			//20141002不准备设置NULLMsg了，因为0xFF会发送不出去
			//if(*pNewMsg != NULLMsg)	//看收到的消息是不是NULLMsg,如果是,可以加上这一句不移动消息堆栈指针
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
			EnterGetMsgState();
		}		
	}
	else							//以下用于SendMsg 						   
	{
		if(T1Count == 18)			//等于18时表示已经发送完成
		{
			EnterGetMsgState();		//进入到收消息的模式
		}
		else if(T1Count % 2 == 0)	//逢2个 T/2就发送一位	
		{
			//发送数据
			if(Msg & 0x80)			//要发送的数据为1
			{
				#ifndef USE_P15_FOR_SENDMSG
				State = 1;
				#else
				StateOut = 0;
				#endif
				/*被注释掉的程序是基于以下目的来写的
				如果某一个桩因为线序错误，将COMM通讯线直接接到电源上，而另一个桩恰好拉低，
				则电流有可能短时间达到25mA,超过手册所说的20mA，不过估计短时间内应当没有问题
				*/
				//Set1(P3M1, 2);	
				//Set0(P3M0, 2);
			}
			else						//要发送的数据为0
			{
				#ifndef USE_P15_FOR_SENDMSG
				State = 0;
				#else
				StateOut = 1;
				#endif
				/*被注释掉的程序是基于以下目的来写的
				如果某一个桩因为线序错误，将COMM通讯线直接接到电源上，而另一个桩恰好拉低，
				则电流有可能短时间达到25mA,超过手册所说的20mA，不过估计短时间内应当没有问题
				*/
				//Set0(P3M1, 2);
				//Set1(P3M0, 2);
			}
		
			Msg <<= 1;	
		}		
	}
}

void Int0_Interrupt(void) interrupt 0
{
	//这个中断函数很简单
	//1,进入到这里说明探测到来了消息
	//2,来消息后,马上关闭中断,开启定时器来接收数据,如不关中断,会不断的进入这里
	//3,当消息接收完后,在接收程序中,重新开启外部中断,用于探测下一条消息的到来
	EX0 = 0;				//关闭int0,等一下这个中断服务程序要交出控制权给定定时器在总线上读数据,所以在读数据期间要关闭外部中断
	SendOrGet = GetMsgFlag;	//表示Time1用于接收数据
	T1Count = 0;			//这个是给Time1用的,让Time1中断服务程序知道从int0交控制权给它读数据时是第几次进入Time1中断服务程序
	TR1 = 1;						//开启定时器开始读
}

/*
	20140928写的注释，程序早就存在。下面这段程序是让桩自己插入一个消息，然后驱动自己来工作
	基本上用不到
*/
#ifdef SINGLEDEBUG
#ifdef MAIN_USE
void InsertMsg(uchar TheMsg)
{
	*pNewMsg = TheMsg;		//复制新消息
	//20141002不准备设置NULLMsg了，因为0xFF会发送不出去
	//if(*pNewMsg != NULLMsg)	//看收到的消息是不是NULLMsg,如果是,可以加上这一句不移动消息堆栈指针
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
}
#endif
#endif

void EnterGetMsgState()
{
	//重新进入到接消息模式,接收新消息
	//因为TR1用于发送消息,所以不管原来有没有发送消息,在此关闭这个定时器
	TR1 = 0;

	//要进入取消息模式,则将IO口设置成只读,不去拉低总线电平,也不拉高总线电平
	//将State设置成高，一但在切换成发消息模式中忘记处理IO口状态时，拉低了总线
	//也不至于让其它设备认为有消息了

	State = 1;

	//将IO口P3.2设置成输入状态,这样只接收从总线上得到的信号
	//20141002因为用P15作输出时，P32从来不会改成强推挽，所以在这里也不用改回来
	#ifdef USE_P15_FOR_SENDMSG
	Set1(P1M1, 5);								//设置StateOut为只读
	Set0(P1M0, 5);
	StateOut = 0;
	#else
	//不用P15时，要将P32改为只读
	Set1(P3M1, 2); 
	Set0(P3M0, 2);
	#endif

	//清除中断标志,这是进入中断的一般步骤
	IE0 = 0;

	//打开外部中断,开始接收消息,外部中断只在探测是否有消息时有用
	//一旦探测到有消息,立即关闭外部中断,开启定时器开始读数据
	//读完后清中断,再打开外部中断来探测是否有消息
	EX0 = 1;
}
#endif