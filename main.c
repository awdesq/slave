#include "define.h"
#include "STC12C5A60S2.h"
#include "delay.h"
#include "softreset.h"
#include "useflash.h"
#include "usecommunication.h"
#include "TestFunction.h"
#include <intrins.h>

/*
20121017 将Test10T时间改为10ms,这样一秒只测试100次,这个应该再小一点,可能会更稳定
20121017 将接收头4个改为3个,将发射头旁边的接收头感应取消这样可以不把内部喷黑
		
		为什么要内部喷黑?

		因为如果不为黑,发射头反射到最近接收头上光很强烈,这个接收头接收到很强的红外光,内部AGC电路将接收头增益
		调到了最小,反射桩回送信息时达到这个接收头的光在这个增益下无法形成信号,所以喷黑可以让反射光减到最小,不
		让接收头把增益调这么小

		为什么先遮断光线再慢慢移动到完全移开,接收头还是收不到信号?

		可能原因:遮断时接收头增益调高,慢慢移开,光线处于遮断与没遮断之间,接收头有可能收到到完整38K信号,而接收头把其
		认为是杂讯, 所以增益不断减小,到完全移开物体,接收头增益已经太小,而还认为是杂讯
		现在如果再次完全遮断,接收头原来收到的不完整信号消失,增益再次调高,再移开物体,能很好地收到信号

		Bug:
		慢慢移开会导致认为一直是断的.无法再开始工作,原来设计了一个接收头接地脚用IO口控制,可以给接收头断电源,但现在
		没有这么设计,无法从程序上消除这一Bug
20121113 在桩程序中加入WorkStatus 的工作灯,只要桩处于工作状态,就会将LED1点亮
20121113  在Radiant_38K中加入一个delayms(10),用于等待反射桩开始工作,之前没有这个时序也能正常工作
20121113 在Test10T() 中加入 delayus(10)  5个
20170323 防止意外写入ID
*/

//函数声明
void Init_T0_For_38K();											//初始化用于发送38K方波
uchar Test10T();
uchar StartWork(uchar WorkTime);
void Init_IO_State();												
void Reflect_38K();
void Radiant_38K();
void First_38K();
void CanRestart();
void PositionTest();
//void SelfTest();
void SetIRLedPower(uchar power);
void XunHuanSendMsg();
uchar ReadIRLedPower();
void TestIRRecvTimeLine();

//全局变量
bit 	IsWait;									//IsWait == 1    即不要真的发送38K信号 IsWait == 0	   即要发送38K红外信号
uchar	Count38K;								//发送 脉冲数目*2 的全局变量
uchar	ID;										//用于记录当前的桩类型
uchar	WorkTime = WorkTimeDefault;				//默认工作时间
														
void main()																						  
{
	uchar myMsg;
	ID = IAP_Read_Byte(ID_Addr);				//读取ID
	Init_IO_State();							//初始化ＩＯ口模式
	Init_T0_For_38K();							//初始化T0用于发射38K红外线
	Init_UART_For_Download();					//初始化串口,用于自动下载程序
	InitForCommunication();						//通讯需要
	ClearMsg();									//清除消息,可能是没有必要的

	while(1)									//等待来自主机的测试
	{
		if(GetMsgCount() > 0)					//有消息到来
		{
		//	LED1 = LEDON;
			myMsg = GetMsg();
		   	switch(GetMsgStyle(myMsg))
			{
				//通讯测试:本消息是带ID才有效的消息,收到消息后立即回一个相同的消息
				case COMMUNICATIONTEST:
					if(GetIDFromMsg(myMsg) == ID)
						SendMsg(MergeMsg(ID, COMMUNICATIONTEST));
					break;
				//红外测试:本消息是带ID才有效的消息,收到消息进行一次红外测试,再回一个相同的消息
				case INFRAREDTEST:
					if(GetIDFromMsg(myMsg) == ID)
					{
						if(Test10T() == OPEN) SendMsg(MergeMsg(ID, INFRAREDTEST));
					}
					break;
				//测试结束:有选择性的终止进入正常程序的自测过程,所以也是带ID才有效的消息,这里也是退出前阶段运行,进入正常工作的唯一出口
				case TESTOVER:
					if(GetIDFromMsg(myMsg) == ID)
						goto LableTestOver;
					break;
				//位置测试:不带屏的位置测试,也是带ID的消息,当通断情况发生变化时就会发送一条消息
				case IRPOSITIONTEST:
					if(GetIDFromMsg(myMsg) == ID)
						PositionTest();
					break;
				//自测试消息:当收到消息后,通过3个LED来显示桩的情况
				//case SELFTEST:
				//	SelfTest();
				//	break;
				//重启动消息:连发四条RESTART就会导致从机重启动
				case RESTART:
					CanRestart();
					break;
				//设置工作时间:收到一条STARTWORK消息时,A桩发送光线的时间,当光线被拦断时,工作即停止
				case SETWORKTIME:
					delayms(OneMsgTimeOfms + 1);
					WorkTime = GetMsg();
					if((WorkTime <= WorkTimeMin) || (WorkTime >= WorkTimeMax)) WorkTime = WorkTimeDefault;
					break;
				//改变ID并写入flash的程序,连发四条CHANGEID,会将从机改为消息中所带的ID
				case CHANGEID:
					delayms(OneMsgTimeOfms * 3 + 3);
					if(GetMsgStyle(GetMsg()) == CHANGEID){
					if(GetMsgStyle(GetMsg()) == CHANGEID){
					if(GetMsgStyle(GetMsg()) == CHANGEID){
					if((GetNumFromIDorMsg(myMsg) >= 1) && (GetNumFromIDorMsg(myMsg) <= 6)){
						if(GetMsgCount() == 0){
							IAP_Erase_Sector(0x0000);
							delayms(100);
							IAP_Program_Byte(0x0000, GetIDFromMsg(myMsg));
							ID = GetIDFromMsg(myMsg);
					}}}}}
					break;

				//用于测试红外接收头的时序，多长时间收到信号， 信号持续多长时间
				case USERDEFINE:
					switch(myMsg){
						case IRLEDTIMELINTTEST:
					   		TestIRRecvTimeLine();
							break;

						case MSGTEST:
							LED1 = 0;
							XunHuanSendMsg();
							LED1 = 1;
							break;
					}
					break;
			}
		}
	}
LableTestOver: 
   	if(IsReflect(ID)) 										//反射桩执行
		Reflect_38K();
	else if(IsRadiant(ID) && (GetNumFromIDorMsg(ID) != 1))	//非是1号桩的发射桩,执行
		Radiant_38K();
	else if(IsRadiant(ID) && (GetNumFromIDorMsg(ID) == 1)) 	//1号发射桩执行
		First_38K();									
	while(1);
}

//接收头有信号是0 无信号是1
void Reflect_38K()
{
	//桩			1A		2A		3A		4A		5A		6A		1B		2B		3B		4B		5B		6B
	//启动信号		主机	主机	主机	主机	主机	主机	主机	主机	主机	主机	主机	主机
	//停止信号		自停	自停	自停	自停	自停	自停	1A		2A		3A		4A		5A		6A
	uchar myMsg;
	while(1)
	{
Reflect_START:
		WorkStatus = LEDOFF;	
		while(1)
		{
			myMsg = GetMsg();
			if(DiscardReflectFlag(myMsg ^ MergeMsg(ID, STARTWORK)) == 0)   //收到发射桩或反射桩的启动信号,都会启动该反射桩
			{
				WorkStatus = LEDON;
				break;
			}
			else if(myMsg == RESTART)				//快速连发四条RESTART才会导致重启
			{
				CanRestart();						//继续检查是否可以重启
			}
		}
Reflect_TheNext:
		while((P1 & 0x07) != 0)					//除非3个红外接收头同时有信号,才跳到下一步
		{
			myMsg = GetMsg();
			if(myMsg == MergeMsg(ID, STOPWORK)) 	//收到反射桩的停止信号,停止该反射桩
			{
				goto Reflect_START;
			}
			else if(myMsg == RESTART)				//快速连发四条RESTART才会导致重启
			{
				CanRestart();
			}
		}
		while((P1 & 0x07) != 0x07);				//当3个接收头都没有信号才跳到下一步 

		Count38K = N_38K_WAIT;						//等待
		IsWait = 1;
		TR0 = 1;
		while(TR0);

		Count38K = N_38K;		   					//设置参数,回送红外信号
		IsWait = 0;
		TR0 = 1;
		while(TR0);

		while((P1 & 0x07) != 0x07);				//当3个接收头都没有信号才跳到下一步
		goto Reflect_TheNext;
	}
}

void First_38K()
{
	uchar myMsg, Count;

First_Start:							
	WorkStatus = LEDOFF;					//开始设置成OFF
	while(1)
	{
		myMsg = GetMsg();
		if(myMsg == MergeMsg(ID, STARTWORK))
		{
			WorkStatus = LEDON;				//收到消息设置成ON
			break;
		}
		else if(myMsg == RESTART)			//快速连发四条RESTART才会导致重启
		{
			CanRestart();
		}
	}
	Count = 0;							//这是1A开始工作时的第一个测试,将Count设为0

First_TheNext:
	myMsg = GetMsg();
	if(myMsg == MergeMsg(ID, STOPWORK))
	{
		goto First_Start;
	}
	else if(myMsg == RESTART)				//快速连发四条RESTART才会导致重启
	{
		CanRestart();
	}

	if(Test10T() == OPEN)
	{
		//如果Count > MinCount 说明之前已经有一段时间连续为CLOSE,这时可以发消息给主机运动员已离开1号桩了
		if(Count > Min1ACount)
		{
			SendMsg(MergeMsg(ID, LEAVED));//发送给主机运动员已经LEAVE
			//如果LEAVED消息已发送给主机,从机1A将自己停止工作,一并让1B也停止工作
			//直到主机下一次给1A发送STARTWORK消息才开始工作,这时1B也后在1A的STARTWORK消息驱动下开始工作
			//不必给自己发STOPWORK消息,因为goto First_Start就是让本桩停止工作
			SendMsg(MergeMsg(GetReflectID(1), STOPWORK));
			//回函数的进入点,等下下一次任务
			goto First_Start;	
		}
		else
		{
			//如果Count <= MinCount说明还没有MinCount次连续的CLOSE,则清零,重新计连续CLOSE的次数
			Count = 0;						
		}
	}
	else
	{
		//我每测得一个CLOSE,Count++,但如果期间有一个OPEN,会被置于为0
		//但如果Count已经大于MinCount就不必++了,因为说明已经有一段时间稳定为CLOSE了
		//也就说明运动员的脚已经放稳了
		if(Count <= Min1ACount) 
		{
			Count++;					
		}
		//如果已经大于MinCount,则发送主机已经REACHED的消息,并将Count置为0xFF
		//以后发现Count 为0xFF,就不再发送信息给主机了,也就是REACHED消息也只发送一次
		else if(Count != 0xFF)
		{
			SendMsg(MergeMsg(ID, REACHED));
			Count = 0xFF;
		}
	}
	goto First_TheNext;		
}
//本函数一秒钟可执行584次
//20120716因为改了程序,	这个584次不再准确,需要重新测试,不过应当不会变化多少
//后来经过测试改过来了，应该现在宏定义时间是准确的600次
//20121017将Test10T时间强行设置为10ms一次, 一秒100次,这样将原来一秒钟检测600次变为100次,看能不能解决限幅放大电路的影响
//也有可能将手指慢慢移开变为不遮挡接收头会感应不到这一动作不是限幅电路的作用,而是程序错误
//接收头有信号是0 无信号是1
uchar	Test10T()
{
	uchar SamplingResult;

	//为了让Test10T工作时间为接近10ms，也就是1/100秒
	//StartWork中一次主循环Test10T一次,耗时为1.7ms,所以Test10T应当不足1.7ms
	//以下是强行等待程序片断
	//2013将这一些delay从程序尾部移动到了前部
	delayms(8);			//先延时8ms
	delay200us();		//再延时0.2ms
	delay100us();		//再延时0.1ms
	delay10us();
	delay10us();
	delay10us();
	delay10us();
	delay10us();

	SamplingResult = CLOSE;
	//发送时,本桩也会收到本桩发出的红外信号,当发送完成后,本桩的红外接收头一段时间内仍会有信号输出,在此应等待至红外信号全部消失
	Count38K = N_38K;									//发送,直到发送完成 
	IsWait = 0;
	TR0 = 1;
	while(TR0);	
	//while((P1 & 0x0F) != 0x0F);						//排除自己干扰
	while((P1 & 0x07) != 0x07);							//排除自己干扰

	Count38K = N_38K_WAIT;								//等待一段时间再取样
	IsWait = 1;
	TR0 = 1;
	while(TR0);

	Count38K = N_38K_SAMPLING;				 			//取样一段时间
	IsWait = 1;
	TR0 = 1;
	while(TR0)
		if((P1 & 0x07) == 0) SamplingResult = OPEN;		//取样过程中有一次全有信号,认为是OPEN的(桩之间是开的,没遮挡)

	while((P1 & 0x07) != 0x07);							//有可能取样完成后,接收头的信号仍然有,在此等待到结束
		
	Count38K = N_38K_WAIT;									//再等待一段时间
	IsWait = 1;
	TR0 = 1;
	while(TR0);

	return SamplingResult;
}

void Radiant_38K()
{
	uchar Result, myMsg;
	while(1)
	{
		if(GetMsgCount() > 0)
		{
			myMsg = GetMsg();
			if(myMsg == MergeMsg(ID, STARTWORK)) 							//是让本桩开始工作才执行
			{
				WorkStatus = LEDON;
				Result = StartWork(WorkTime);								//开成一段时间光线进行测试,注意StartWork会自己发消息
				if(Result == REACHED)										//到达了这个桩
				{
					SendMsg(MergeMsg(ID, REACHED));							//通知主机
				}
				else if(Result == TIMEOVER)									//超时
				{
					SendMsg(MergeMsg(ID, TIMEOVER));						//通知主机
				}
				SendMsg(MergeMsg(GetReflectID(ID), STOPWORK));				//只要执行完StartWork(WorkTime),2 3 4 5 6 就停止自己反射桩的工作
				WorkStatus = LEDOFF;
			}
			else if(myMsg == RESTART)										//快速连发四条RESTART才会导致重启
			{
				CanRestart();
			}
		}		
	}
} 

//最长工作时间 255 * 0.1s = 25.5秒
uchar StartWork(uchar t_100ms)
{
	uint i;
	uchar myMsg;
	for(i = 0; i < (uint)t_100ms * (uint)TEST_TIMES_100MS ; i++)
	{
		myMsg = GetMsg();
		if(myMsg == MergeMsg(ID, STOPWORK))
		{
			return INTERRUPT_BY_CODE;
		}
		else if(myMsg == RESTART)			//快速连发四条RESTART才会导致重启
		{
		 	CanRestart();
		}

		if(Test10T() == CLOSE)
		{
			return REACHED;
		}
	}
	return TIMEOVER;
}

void Init_IO_State()
{
	//关闭所有IR Led 的输出
	SetIRLedPower(0);

	//根据ID号,启用不同的端口成强推挽,设置成不同的功率
	if(GetNumFromIDorMsg(ID) == 6)
	{
		SetIRLedPower(4);							//最大功率
	}
	else if(GetNumFromIDorMsg(ID) == 3)
	{
		SetIRLedPower(2);							//3号桩的功率
	}else if((GetNumFromIDorMsg(ID) == 1) || \
			 (GetNumFromIDorMsg(ID) == 2) || \
			 (GetNumFromIDorMsg(ID) == 4) || \
			 (GetNumFromIDorMsg(ID) == 5))
	{
		SetIRLedPower(1);							 //最小功率
	}

	//将IR_REC_1,2,3设置为只读
	Set1(P1M1, 0);								
	Set0(P1M0, 0);	
 	Set1(P1M1, 1);
	Set0(P1M0, 1);
	Set1(P1M1, 2);
	Set0(P1M0, 2);
}

void Init_T0_For_38K()
{	
	EA = 0;						//中断总控制器关	
	TMOD |= 0x02;				//定时器工作方式2,8位自动重载	
	AUXR |= 0x80;				//不12分频设置,让定时器工作足够快	
	ET0 = 1;					//0号定时器允许	
	TH0 = Val_TH0_Half;			//初始化TH0	
	EA = 1;						//中断总控制器开
}

/*
	定时器0的服务程序 38K方波产生程序
	因为有可能主程序只是要通过这个中断来计算等待时间，就是说在这段时间内并不发送红
	外信号，所以要引入一个变量IsWait来判断
	IsWait == 1    即不要真的发送信号
	IsWait == 0	   即要发送信号
*/
void Time0Interrupt() interrupt 1
{
	//38K发送程序
	if(Count38K-- > 0)				//半脉冲数Count38K大于0就继续发送,否则关闭定时器
	{
		if(IsWait == 0)
		{		
			IR_LED_TX1 = !IR_LED_TX1;
			IR_LED_TX2 = !IR_LED_TX2;
			IR_LED_TX3 = !IR_LED_TX3;
			IR_LED_TX4 = !IR_LED_TX4;
		}			
	}
	else
	{
		TR0 = 0;
		IR_LED_TX1 = IR_LED_OFF;
		IR_LED_TX2 = IR_LED_OFF;
		IR_LED_TX3 = IR_LED_OFF;
		IR_LED_TX4 = IR_LED_OFF;
	}
}

void CanRestart()
{
	//暂停一段时间，时间长度：3个消息时间+3ms
	delayms(OneMsgTimeOfms * 3 + 3);
	//连续收到3个RESTART信息才重启从机
	if(GetMsg() == RESTART)
	{
		if(GetMsg() == RESTART)
		{
			if(GetMsg() == RESTART)	
			{
				SoftResetToISPMonitor();
			}
		}
	}
}

void PositionTest()
{
	uchar myMsg, PrevStatus;

	WorkStatus = LEDOFF;					//开始设置成OFF
	while(1)
	{
		myMsg = GetMsg();
		if(myMsg == MergeMsg(ID, STARTWORK))
		{
			WorkStatus = LEDON;				//收到消息设置成ON
			delayms(10);					//等待反射桩开始工作
			break;
		}
		else if(myMsg == RESTART)			//快速连发四条RESTART才会导致重启
		{
			CanRestart();
		}
	}
	PrevStatus = Test10T();
	if(PrevStatus == CLOSE)
	{
		SendMsg(MergeMsg(ID, REACHED));
	}

PositionTest_TheNext:
	myMsg = GetMsg();
	if(myMsg == MergeMsg(ID, STOPWORK))		//收到停止工作时,返回主循环
	{
		return;
	}
	else if(myMsg == RESTART)				//快速连发四条RESTART才会导致重启
	{
		CanRestart();
	}

	if(Test10T() != PrevStatus)
	{
		if(PrevStatus == CLOSE)				//前一个状态CLOSE状态,意味着这个状态为OPEN
		{
			SendMsg(MergeMsg(ID, LEAVED));	//发送信号 由CLOSE 变为 OPEN
			PrevStatus = OPEN;				//保存此时的状态
		}
		else								//前一个状态为OPEN,意味着这个状态为CLOSE
		{
			SendMsg(MergeMsg(ID, REACHED));//发送信号 由OPEN 变为　CLOSE
			PrevStatus = CLOSE;				//保存现在的状态
		}
	}
	goto PositionTest_TheNext;		
}



/*
	设置IR_LED发光的功率
	可用参数
	 0 										不发射
	 1 	TX4	 P1.6	限流电阻：470 + 10欧	1号 2号 4号 5号
	 2	TX3	 P1.5	限流电阻：390 + 10欧	3号
	 3	TX2	 P1.4	限流电阻：200 + 10欧	极端情况备用
	 4	TX1	 P1.3	限流电阻：0   + 10欧	6号
*/
void SetIRLedPower(uchar power){
	//将4个IO口都设置成只读
	P1 |= 0x78;			//将P1的 3 4 5 6 位设置成高，这样不会触发红外信号
	P1M1 |= 0x78;		//将P1M1的 3 4 5 6 位设置成1
	P1M0 &= 0x87;		//将P1M0的 3 4 5 6 位设置成0

	switch(power)
	{
		//设置成0，不发射红外光线
		case 0:
			break;

		//根据需要设置某一个IO口为强推
		case 1:
		case 2:
		case 3:
		case 4:
			Set0(P1M1, 7 - power);
			Set1(P1M0, 7 - power);
			break;

		//如果参数不对，设置成不发射
		default:
			break;
	}
}

uchar ReadIRLedPower(){
	//读入P1M1
	uchar M1 = P1M1;
	
	//右移三位，得到P1M1右移三位的值以便于判断
	//P1.0 P1.1 P1.2 		是红外接收头
	//P1.3 P1.4 P1.5 P1.6	是红外发射头
	//不发射红外线 				最大功率
	//P1M1 X111 1XXX			X111 0XXX
	//P1M0 X000 0XXX			X111 1XXX
	M1 = (M1 >> 3) & 0x0F;
	
	switch(M1){
		case 0x0E:
			return 4;
		
		case 0x0D:
			return 3;
		
		case 0x0B:
			return 2;
		
		case 0x07:
			return 1;
		
		case 0x0F:
			return 0;
		
		default:
			return 0xFF;
	}
}

/*
	1、设置定时器，1us中断一次，并设置一个全局变量，以让中断程序知道跑哪一段
	2、发送红外信号，并开始计时
	3、收到信号时记录
	4、信号消失时记录
	5、给中断程序里有的全局变量设置值，让其给主机发送信息，因为同步通讯也要用到TIME1
	6、重复以上步骤多次测量
*/
void TestIRRecvTimeLine(){
	uchar i, power, tmp;
	uchar start_H, start_L, end_H, end_L;
	uchar code TAB[3] = {0x01, 0x02, 0x04};
	
	//等待一条消息的时间
	delayms(OneMsgTimeOfms);

	//如果收到了用哪个功率的消息，则设置相应功率
	//否则退出
	if(GetMsgCount() == 0){
		return;
	}

	//得到消息
	tmp = GetMsg();

	//验证消息,不正确时退出
	if(!(tmp >= 0 && tmp <= 4)){
		return;
	}

	//存储原来的功率
	power = ReadIRLedPower();

	//设置新功率
	SetIRLedPower(tmp);

	i = 0;
	//轮流为每个接收头测试一次
	while(i < 3){
	
		//初始化变量
		start_H = start_L = end_H = end_L = 0;
	
		//设置中断
		AUXR &= 0xBF;		//定时器时钟12T模式
		TMOD &= 0x0F;		//设置定时器模式
		TMOD |= 0x10;		//设置定时器模式
		TL1 = 0x00;			//设置定时初值
		TH1 = 0x00;			//设置定时初值
		TF1 = 0;			//清除TF1标志
		
		//发送几个周期，默认为10个周期，详见define.h
		Count38K = N_38K;									//发送,直到发送完成 
		IsWait = 0;
		TR0 = 1;
		TR1 = 1;		//定时器1开始计时
	
		tmp = TAB[i];		//2016.4.20从下方移动到循环外
		while(1){
			//判断是否开始接收到信号
			//还从未接收到信号 
			//tmp = TAB[i];		   //2016.4.20原位置
			if(start_L == 0 && start_H == 0){
				//一旦接收到信号就记录下start
				if((P1 & tmp) == 0){
					start_H = TH1;
					start_L = TL1;
					LED1 = LEDON;
				}
			}else if(end_L == 0 && end_H == 0){
				if((P1 & tmp) == tmp){		 //2016.4.20修复一个bug,原来是 if(P1 & tmp == tmp){
					end_H = TH1;
					end_L = TL1;
					TR1 = 0;
					TF1 = 0;
					LED1 = LEDOFF;
					break;
				}
			}
			
			if(TH1 > 250){
				TR1 = 0;
				TF1 = 0;
				break;
			}
		}

		//恢复定时器的同步通讯功能
		AUXR |= 0x40;						//定时器时钟1T模式
		TMOD &= 0x0F;						//设置定时器模式
		TMOD |= 0x10;						//设置定时器模式
	    TH1 = COMMUNICATION_TH1;			//初值，设置见UseCommunication中
	    TL1 = COMMUNICATION_TL1;			//初值，设置见UseCommunication中
		ET1 = 1;							//定时器1允许位

		//发送计数器所获得的数据
		SendMsg(start_H);
		SendMsg(start_L);
		SendMsg(end_H);
		SendMsg(end_L);
		i++;
		//因为要共用计时器，所以一定要等SendMsg用完计时器再开始下一轮测试
		while(TR1 == 1);
	}

	//发送本从机ID
	SendMsg(ID);
	while(TR1 == 1);

	//恢复旧功率
	SetIRLedPower(power);
}

void XunHuanSendMsg(){
	uint i = 1;
	delayms(500);

	//给一个退出的条件
	while(i){
		//闪烁LED
		LED1 = !LED1;
		SendMsg(i++);
	}
}