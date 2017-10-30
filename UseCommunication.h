#include "stc12c5a60s2.h"
#include "define.h"

#ifndef	USECOMMULICATION
#define	USECOMMULICATION

//����ֻ�����Ƕ��е���һ��������PNP�Ĺ���,�Ժ��ʮ����������NPN�Ĺ���
//��ǰ��������NPN��PNP��������,����ֱ����IO������.
//���˵���汾��׮ֱ����IO������,���Σ�վ��ǰ�IO�ڱ�¶����,����ԭ������ʱ��Ҳ�ǽ�IO�ڱ�¶����
//�����׮�����иĶ��ĳ��򽫲���������������
//��ֻ��һ��IO��,P3.2��������Ϣ�ͽ�����Ϣ,�൱������PNP�Ĺ���,������Щ���ó�USE_PNP
//20140927���һ��׮��û��ʹ�������������͵�ƽ����ֱ����IO�����͵�ƽ��׼������Ҳʹ����ͬ������
//���Ա������ֻ��Ӧ����׮��ֻ��һ��IRLED��׮����һЩ����ĳ���һ��ɾ����

#define SLAVE_USE								//�ӻ�ͨ��ר��
//#define MAIN_USE								//����ͨѶ��
//#define SINGLEDEBUG							//��һ�������������г����õ�һ������
//#define USE_P15_FOR_SENDMSG						//�����P1.5��ͨѶ�Ļ����붨�������


sbit	State = P3^2;							//������ⲿ�жϵ�IO��
#ifdef USE_P15_FOR_SENDMSG
sbit	StateOut = P1^5;
#endif

#define	MsgBufferLen	30						//����������
#define	GetMsgFlag		0
#define	SendMsgFlag		1
#define NULLMsg			0xFF
#define	SUCCESS			1						//���ͳɹ��ķ���ֵ
#define FAIL			0						//����ʧ�ܵķ���ֵ
#define	COMMUNICATION_TH1	0xFB				//ͨѶʱTH1��ֵ	   200usһλ 100usһ���ж�
#define COMMUNICATION_TL1	0x50				//ͨѶʱTL1��ֵ
#define OneMsgTimeOfms	2		
//ID��ȷ��
//FLASH��0000��Ϊ��׮��ID,
//��3λΪ׮��� 1 2 3 4 5 6 
//������4λΪ0��ʾ����׮,Ϊ1��ʾ�����
//ÿһ����Ϣ�ĵ���λΪ׮ID
//ÿһ����Ϣ�ĸ���λΪ��Ϣ����,����λΪID
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
#define GetReflectID(ID)			(((ID) | 0x08) & 0x0F)			//������׮ID��Ϊ����׮ID
#define	IsRadiant(ID_or_Msg)		(((ID_or_Msg) & 0x08) == 0)		//��4λΪ0�Ƿ���׮
#define IsReflect(ID_or_Msg)		(((ID_or_Msg) & 0x08) == 0x08)	//��4λΪ1�Ƿ����
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