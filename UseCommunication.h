#include "stc12c5a60s2.h"
#include "define.h"

#ifndef	USECOMMULICATION
#define	USECOMMULICATION

//好像只有望城二中的那一套仪器是PNP的管子,以后的十套仪器都是NPN的管子
//以前的拉低用NPN或PNP管子拉低,现在直接用IO口拉低.
//到了第五版本的桩直接用IO口拉低,最大危险就是把IO口暴露在外,不过原来读的时候也是将IO口暴露在外
//这个在桩程序中改动的程序将不再适用于主机中
//当只用一个IO口,P3.2来发送消息和接收消息,相当于用了PNP的管子,可以在些设置成USE_PNP
//20140927最后一批桩中没有使用三极管来拉低电平，而直接用IO口拉低电平，准备主机也使用相同的设置
//所以本版程序只适应于新桩，只有一个IRLED的桩，将一些冗余的程序一并删除。

#define SLAVE_USE								//从机通读专用
//#define MAIN_USE								//主机通讯用
//#define SINGLEDEBUG							//用一个主机测试所有程序用到一个函数
//#define USE_P15_FOR_SENDMSG						//如果用P1.5来通讯的话，请定义这个宏


sbit	State = P3^2;							//这个是外部中断的IO口
#ifdef USE_P15_FOR_SENDMSG
sbit	StateOut = P1^5;
#endif

#define	MsgBufferLen	30						//缓冲区长度
#define	GetMsgFlag		0
#define	SendMsgFlag		1
#define NULLMsg			0xFF
#define	SUCCESS			1						//发送成功的返回值
#define FAIL			0						//发送失败的返回值
#define	COMMUNICATION_TH1	0xFB				//通讯时TH1初值	   200us一位 100us一次中断
#define COMMUNICATION_TL1	0x50				//通讯时TL1初值
#define OneMsgTimeOfms	2		
//ID的确定
//FLASH的0000处为本桩的ID,
//低3位为桩序号 1 2 3 4 5 6 
//倒数第4位为0表示发射桩,为1表示反射村
//每一个消息的低四位为桩ID
//每一个消息的高四位为消息类型,低四位为ID
#define STARTWORK					0x00
#define	REACHED						0x10
#define	LEAVED						0x20
#define	TIMEOVER					0x30
#define SETWORKTIME					0x40
#define COMMUNICATIONTEST			0x50
#define	INFRAREDTEST				0x60	
#define TESTOVER					0x70
#define CHANGEID					0x80
#define STOPWORK					0x90
#define	RESTART						0xA0
#define	IRPOSITIONTEST				0xB0
#define	SELFTEST					0xC0
#define	USERDEFINE					0xD0
#define	IRLEDTIMELINTTEST			0xD1
#define MSGTEST						0xD2
#define	NO_USE5						0xE0
#define	NO_USE6						0xF0
#define GetReflectID(ID)			(((ID) | 0x08) & 0x0F)			//将发射桩ID变为反射桩ID
#define	IsRadiant(ID_or_Msg)		(((ID_or_Msg) & 0x08) == 0)		//第4位为0是发射桩
#define IsReflect(ID_or_Msg)		(((ID_or_Msg) & 0x08) == 0x08)	//第4位为1是反射村
#define MergeMsg(ID, MsgStyle) 		((ID) | (MsgStyle))
#define DiscardReflectFlag(ID)		((ID) & 0xF7)
#define GetMsgStyle(Msg)			((Msg) & 0xF0)
#define GetNumFromIDorMsg(IDorMsg)	((IDorMsg) & 0x07)
#define GetIDFromMsg(Msg)			((Msg) & 0x0F)

void InitForCommunication(void);
uchar GetMsgCount();
uchar GetMsg();
uchar ClearMsg();
uchar SendMsg(uchar myMsg);
void EnterGetMsgState();

#ifdef SINGLEDEBUG
void  InsertMsg(uchar TheMsg);
#endif
#endif