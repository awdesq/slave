#include "define.h"
#include "STC12C5A60S2.h"
#include "delay.h"
#include "softreset.h"
#include "useflash.h"
#include "usecommunication.h"
#include "TestFunction.h"
#include <intrins.h>

/*
20121017 ��Test10Tʱ���Ϊ10ms,����һ��ֻ����100��,���Ӧ����Сһ��,���ܻ���ȶ�
20121017 ������ͷ4����Ϊ3��,������ͷ�ԱߵĽ���ͷ��Ӧȡ���������Բ����ڲ����
		
		ΪʲôҪ�ڲ����?

		��Ϊ�����Ϊ��,����ͷ���䵽�������ͷ�Ϲ��ǿ��,�������ͷ���յ���ǿ�ĺ����,�ڲ�AGC��·������ͷ����
		��������С,����׮������Ϣʱ�ﵽ�������ͷ�Ĺ�������������޷��γ��ź�,������ڿ����÷���������С,��
		�ý���ͷ���������ôС

		Ϊʲô���ڶϹ����������ƶ�����ȫ�ƿ�,����ͷ�����ղ����ź�?

		����ԭ��:�ڶ�ʱ����ͷ�������,�����ƿ�,���ߴ����ڶ���û�ڶ�֮��,����ͷ�п����յ�������38K�ź�,������ͷ����
		��Ϊ����Ѷ, �������治�ϼ�С,����ȫ�ƿ�����,����ͷ�����Ѿ�̫С,������Ϊ����Ѷ
		��������ٴ���ȫ�ڶ�,����ͷԭ���յ��Ĳ������ź���ʧ,�����ٴε���,���ƿ�����,�ܺܺõ��յ��ź�

		Bug:
		�����ƿ��ᵼ����Ϊһֱ�Ƕϵ�.�޷��ٿ�ʼ����,ԭ�������һ������ͷ�ӵؽ���IO�ڿ���,���Ը�����ͷ�ϵ�Դ,������
		û����ô���,�޷��ӳ�����������һBug
20121113 ��׮�����м���WorkStatus �Ĺ�����,ֻҪ׮���ڹ���״̬,�ͻὫLED1����
20121113  ��Radiant_38K�м���һ��delayms(10),���ڵȴ�����׮��ʼ����,֮ǰû�����ʱ��Ҳ����������
20121113 ��Test10T() �м��� delayus(10)  5��
20170323 ��ֹ����д��ID
*/

//��������
void Init_T0_For_38K();											//��ʼ�����ڷ���38K����
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

//ȫ�ֱ���
bit 	IsWait;									//IsWait == 1    ����Ҫ��ķ���38K�ź� IsWait == 0	   ��Ҫ����38K�����ź�
uchar	Count38K;								//���� ������Ŀ*2 ��ȫ�ֱ���
uchar	ID;										//���ڼ�¼��ǰ��׮����
uchar	WorkTime = WorkTimeDefault;				//Ĭ�Ϲ���ʱ��
														
void main()																						  
{
	uchar myMsg;
	ID = IAP_Read_Byte(ID_Addr);				//��ȡID
	Init_IO_State();							//��ʼ���ɣϿ�ģʽ
	Init_T0_For_38K();							//��ʼ��T0���ڷ���38K������
	Init_UART_For_Download();					//��ʼ������,�����Զ����س���
	InitForCommunication();						//ͨѶ��Ҫ
	ClearMsg();									//�����Ϣ,������û�б�Ҫ��

	while(1)									//�ȴ����������Ĳ���
	{
		if(GetMsgCount() > 0)					//����Ϣ����
		{
		//	LED1 = LEDON;
			myMsg = GetMsg();
		   	switch(GetMsgStyle(myMsg))
			{
				//ͨѶ����:����Ϣ�Ǵ�ID����Ч����Ϣ,�յ���Ϣ��������һ����ͬ����Ϣ
				case COMMUNICATIONTEST:
					if(GetIDFromMsg(myMsg) == ID)
						SendMsg(MergeMsg(ID, COMMUNICATIONTEST));
					break;
				//�������:����Ϣ�Ǵ�ID����Ч����Ϣ,�յ���Ϣ����һ�κ������,�ٻ�һ����ͬ����Ϣ
				case INFRAREDTEST:
					if(GetIDFromMsg(myMsg) == ID)
					{
						if(Test10T() == OPEN) SendMsg(MergeMsg(ID, INFRAREDTEST));
					}
					break;
				//���Խ���:��ѡ���Ե���ֹ��������������Բ����,����Ҳ�Ǵ�ID����Ч����Ϣ,����Ҳ���˳�ǰ�׶�����,��������������Ψһ����
				case TESTOVER:
					if(GetIDFromMsg(myMsg) == ID)
						goto LableTestOver;
					break;
				//λ�ò���:��������λ�ò���,Ҳ�Ǵ�ID����Ϣ,��ͨ����������仯ʱ�ͻᷢ��һ����Ϣ
				case IRPOSITIONTEST:
					if(GetIDFromMsg(myMsg) == ID)
						PositionTest();
					break;
				//�Բ�����Ϣ:���յ���Ϣ��,ͨ��3��LED����ʾ׮�����
				//case SELFTEST:
				//	SelfTest();
				//	break;
				//��������Ϣ:��������RESTART�ͻᵼ�´ӻ�������
				case RESTART:
					CanRestart();
					break;
				//���ù���ʱ��:�յ�һ��STARTWORK��Ϣʱ,A׮���͹��ߵ�ʱ��,�����߱�����ʱ,������ֹͣ
				case SETWORKTIME:
					delayms(OneMsgTimeOfms + 1);
					WorkTime = GetMsg();
					if((WorkTime <= WorkTimeMin) || (WorkTime >= WorkTimeMax)) WorkTime = WorkTimeDefault;
					break;
				//�ı�ID��д��flash�ĳ���,��������CHANGEID,�Ὣ�ӻ���Ϊ��Ϣ��������ID
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

				//���ڲ��Ժ������ͷ��ʱ�򣬶೤ʱ���յ��źţ� �źų����೤ʱ��
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
   	if(IsReflect(ID)) 										//����׮ִ��
		Reflect_38K();
	else if(IsRadiant(ID) && (GetNumFromIDorMsg(ID) != 1))	//����1��׮�ķ���׮,ִ��
		Radiant_38K();
	else if(IsRadiant(ID) && (GetNumFromIDorMsg(ID) == 1)) 	//1�ŷ���׮ִ��
		First_38K();									
	while(1);
}

//����ͷ���ź���0 ���ź���1
void Reflect_38K()
{
	//׮			1A		2A		3A		4A		5A		6A		1B		2B		3B		4B		5B		6B
	//�����ź�		����	����	����	����	����	����	����	����	����	����	����	����
	//ֹͣ�ź�		��ͣ	��ͣ	��ͣ	��ͣ	��ͣ	��ͣ	1A		2A		3A		4A		5A		6A
	uchar myMsg;
	while(1)
	{
Reflect_START:
		WorkStatus = LEDOFF;	
		while(1)
		{
			myMsg = GetMsg();
			if(DiscardReflectFlag(myMsg ^ MergeMsg(ID, STARTWORK)) == 0)   //�յ�����׮����׮�������ź�,���������÷���׮
			{
				WorkStatus = LEDON;
				break;
			}
			else if(myMsg == RESTART)				//������������RESTART�Żᵼ������
			{
				CanRestart();						//��������Ƿ��������
			}
		}
Reflect_TheNext:
		while((P1 & 0x07) != 0)					//����3���������ͷͬʱ���ź�,��������һ��
		{
			myMsg = GetMsg();
			if(myMsg == MergeMsg(ID, STOPWORK)) 	//�յ�����׮��ֹͣ�ź�,ֹͣ�÷���׮
			{
				goto Reflect_START;
			}
			else if(myMsg == RESTART)				//������������RESTART�Żᵼ������
			{
				CanRestart();
			}
		}
		while((P1 & 0x07) != 0x07);				//��3������ͷ��û���źŲ�������һ�� 

		Count38K = N_38K_WAIT;						//�ȴ�
		IsWait = 1;
		TR0 = 1;
		while(TR0);

		Count38K = N_38K;		   					//���ò���,���ͺ����ź�
		IsWait = 0;
		TR0 = 1;
		while(TR0);

		while((P1 & 0x07) != 0x07);				//��3������ͷ��û���źŲ�������һ��
		goto Reflect_TheNext;
	}
}

void First_38K()
{
	uchar myMsg, Count;

First_Start:							
	WorkStatus = LEDOFF;					//��ʼ���ó�OFF
	while(1)
	{
		myMsg = GetMsg();
		if(myMsg == MergeMsg(ID, STARTWORK))
		{
			WorkStatus = LEDON;				//�յ���Ϣ���ó�ON
			break;
		}
		else if(myMsg == RESTART)			//������������RESTART�Żᵼ������
		{
			CanRestart();
		}
	}
	Count = 0;							//����1A��ʼ����ʱ�ĵ�һ������,��Count��Ϊ0

First_TheNext:
	myMsg = GetMsg();
	if(myMsg == MergeMsg(ID, STOPWORK))
	{
		goto First_Start;
	}
	else if(myMsg == RESTART)				//������������RESTART�Żᵼ������
	{
		CanRestart();
	}

	if(Test10T() == OPEN)
	{
		//���Count > MinCount ˵��֮ǰ�Ѿ���һ��ʱ������ΪCLOSE,��ʱ���Է���Ϣ�������˶�Ա���뿪1��׮��
		if(Count > Min1ACount)
		{
			SendMsg(MergeMsg(ID, LEAVED));//���͸������˶�Ա�Ѿ�LEAVE
			//���LEAVED��Ϣ�ѷ��͸�����,�ӻ�1A���Լ�ֹͣ����,һ����1BҲֹͣ����
			//ֱ��������һ�θ�1A����STARTWORK��Ϣ�ſ�ʼ����,��ʱ1BҲ����1A��STARTWORK��Ϣ�����¿�ʼ����
			//���ظ��Լ���STOPWORK��Ϣ,��Ϊgoto First_Start�����ñ�׮ֹͣ����
			SendMsg(MergeMsg(GetReflectID(1), STOPWORK));
			//�غ����Ľ����,������һ������
			goto First_Start;	
		}
		else
		{
			//���Count <= MinCount˵����û��MinCount��������CLOSE,������,���¼�����CLOSE�Ĵ���
			Count = 0;						
		}
	}
	else
	{
		//��ÿ���һ��CLOSE,Count++,������ڼ���һ��OPEN,�ᱻ����Ϊ0
		//�����Count�Ѿ�����MinCount�Ͳ���++��,��Ϊ˵���Ѿ���һ��ʱ���ȶ�ΪCLOSE��
		//Ҳ��˵���˶�Ա�Ľ��Ѿ�������
		if(Count <= Min1ACount) 
		{
			Count++;					
		}
		//����Ѿ�����MinCount,���������Ѿ�REACHED����Ϣ,����Count��Ϊ0xFF
		//�Ժ���Count Ϊ0xFF,�Ͳ��ٷ�����Ϣ��������,Ҳ����REACHED��ϢҲֻ����һ��
		else if(Count != 0xFF)
		{
			SendMsg(MergeMsg(ID, REACHED));
			Count = 0xFF;
		}
	}
	goto First_TheNext;		
}
//������һ���ӿ�ִ��584��
//20120716��Ϊ���˳���,	���584�β���׼ȷ,��Ҫ���²���,����Ӧ������仯����
//�����������ԸĹ����ˣ�Ӧ�����ں궨��ʱ����׼ȷ��600��
//20121017��Test10Tʱ��ǿ������Ϊ10msһ��, һ��100��,������ԭ��һ���Ӽ��600�α�Ϊ100��,���ܲ��ܽ���޷��Ŵ��·��Ӱ��
//Ҳ�п��ܽ���ָ�����ƿ���Ϊ���ڵ�����ͷ���Ӧ������һ���������޷���·������,���ǳ������
//����ͷ���ź���0 ���ź���1
uchar	Test10T()
{
	uchar SamplingResult;

	//Ϊ����Test10T����ʱ��Ϊ�ӽ�10ms��Ҳ����1/100��
	//StartWork��һ����ѭ��Test10Tһ��,��ʱΪ1.7ms,����Test10TӦ������1.7ms
	//������ǿ�еȴ�����Ƭ��
	//2013����һЩdelay�ӳ���β���ƶ�����ǰ��
	delayms(8);			//����ʱ8ms
	delay200us();		//����ʱ0.2ms
	delay100us();		//����ʱ0.1ms
	delay10us();
	delay10us();
	delay10us();
	delay10us();
	delay10us();

	SamplingResult = CLOSE;
	//����ʱ,��׮Ҳ���յ���׮�����ĺ����ź�,��������ɺ�,��׮�ĺ������ͷһ��ʱ�����Ի����ź����,�ڴ�Ӧ�ȴ��������ź�ȫ����ʧ
	Count38K = N_38K;									//����,ֱ��������� 
	IsWait = 0;
	TR0 = 1;
	while(TR0);	
	//while((P1 & 0x0F) != 0x0F);						//�ų��Լ�����
	while((P1 & 0x07) != 0x07);							//�ų��Լ�����

	Count38K = N_38K_WAIT;								//�ȴ�һ��ʱ����ȡ��
	IsWait = 1;
	TR0 = 1;
	while(TR0);

	Count38K = N_38K_SAMPLING;				 			//ȡ��һ��ʱ��
	IsWait = 1;
	TR0 = 1;
	while(TR0)
		if((P1 & 0x07) == 0) SamplingResult = OPEN;		//ȡ����������һ��ȫ���ź�,��Ϊ��OPEN��(׮֮���ǿ���,û�ڵ�)

	while((P1 & 0x07) != 0x07);							//�п���ȡ����ɺ�,����ͷ���ź���Ȼ��,�ڴ˵ȴ�������
		
	Count38K = N_38K_WAIT;									//�ٵȴ�һ��ʱ��
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
			if(myMsg == MergeMsg(ID, STARTWORK)) 							//���ñ�׮��ʼ������ִ��
			{
				WorkStatus = LEDON;
				Result = StartWork(WorkTime);								//����һ��ʱ����߽��в���,ע��StartWork���Լ�����Ϣ
				if(Result == REACHED)										//���������׮
				{
					SendMsg(MergeMsg(ID, REACHED));							//֪ͨ����
				}
				else if(Result == TIMEOVER)									//��ʱ
				{
					SendMsg(MergeMsg(ID, TIMEOVER));						//֪ͨ����
				}
				SendMsg(MergeMsg(GetReflectID(ID), STOPWORK));				//ֻҪִ����StartWork(WorkTime),2 3 4 5 6 ��ֹͣ�Լ�����׮�Ĺ���
				WorkStatus = LEDOFF;
			}
			else if(myMsg == RESTART)										//������������RESTART�Żᵼ������
			{
				CanRestart();
			}
		}		
	}
} 

//�����ʱ�� 255 * 0.1s = 25.5��
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
		else if(myMsg == RESTART)			//������������RESTART�Żᵼ������
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
	//�ر�����IR Led �����
	SetIRLedPower(0);

	//����ID��,���ò�ͬ�Ķ˿ڳ�ǿ����,���óɲ�ͬ�Ĺ���
	if(GetNumFromIDorMsg(ID) == 6)
	{
		SetIRLedPower(4);							//�����
	}
	else if(GetNumFromIDorMsg(ID) == 3)
	{
		SetIRLedPower(2);							//3��׮�Ĺ���
	}else if((GetNumFromIDorMsg(ID) == 1) || \
			 (GetNumFromIDorMsg(ID) == 2) || \
			 (GetNumFromIDorMsg(ID) == 4) || \
			 (GetNumFromIDorMsg(ID) == 5))
	{
		SetIRLedPower(1);							 //��С����
	}

	//��IR_REC_1,2,3����Ϊֻ��
	Set1(P1M1, 0);								
	Set0(P1M0, 0);	
 	Set1(P1M1, 1);
	Set0(P1M0, 1);
	Set1(P1M1, 2);
	Set0(P1M0, 2);
}

void Init_T0_For_38K()
{	
	EA = 0;						//�ж��ܿ�������	
	TMOD |= 0x02;				//��ʱ��������ʽ2,8λ�Զ�����	
	AUXR |= 0x80;				//��12��Ƶ����,�ö�ʱ�������㹻��	
	ET0 = 1;					//0�Ŷ�ʱ������	
	TH0 = Val_TH0_Half;			//��ʼ��TH0	
	EA = 1;						//�ж��ܿ�������
}

/*
	��ʱ��0�ķ������ 38K������������
	��Ϊ�п���������ֻ��Ҫͨ������ж�������ȴ�ʱ�䣬����˵�����ʱ���ڲ������ͺ�
	���źţ�����Ҫ����һ������IsWait���ж�
	IsWait == 1    ����Ҫ��ķ����ź�
	IsWait == 0	   ��Ҫ�����ź�
*/
void Time0Interrupt() interrupt 1
{
	//38K���ͳ���
	if(Count38K-- > 0)				//��������Count38K����0�ͼ�������,����رն�ʱ��
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
	//��ͣһ��ʱ�䣬ʱ�䳤�ȣ�3����Ϣʱ��+3ms
	delayms(OneMsgTimeOfms * 3 + 3);
	//�����յ�3��RESTART��Ϣ�������ӻ�
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

	WorkStatus = LEDOFF;					//��ʼ���ó�OFF
	while(1)
	{
		myMsg = GetMsg();
		if(myMsg == MergeMsg(ID, STARTWORK))
		{
			WorkStatus = LEDON;				//�յ���Ϣ���ó�ON
			delayms(10);					//�ȴ�����׮��ʼ����
			break;
		}
		else if(myMsg == RESTART)			//������������RESTART�Żᵼ������
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
	if(myMsg == MergeMsg(ID, STOPWORK))		//�յ�ֹͣ����ʱ,������ѭ��
	{
		return;
	}
	else if(myMsg == RESTART)				//������������RESTART�Żᵼ������
	{
		CanRestart();
	}

	if(Test10T() != PrevStatus)
	{
		if(PrevStatus == CLOSE)				//ǰһ��״̬CLOSE״̬,��ζ�����״̬ΪOPEN
		{
			SendMsg(MergeMsg(ID, LEAVED));	//�����ź� ��CLOSE ��Ϊ OPEN
			PrevStatus = OPEN;				//�����ʱ��״̬
		}
		else								//ǰһ��״̬ΪOPEN,��ζ�����״̬ΪCLOSE
		{
			SendMsg(MergeMsg(ID, REACHED));//�����ź� ��OPEN ��Ϊ��CLOSE
			PrevStatus = CLOSE;				//�������ڵ�״̬
		}
	}
	goto PositionTest_TheNext;		
}



/*
	����IR_LED����Ĺ���
	���ò���
	 0 										������
	 1 	TX4	 P1.6	�������裺470 + 10ŷ	1�� 2�� 4�� 5��
	 2	TX3	 P1.5	�������裺390 + 10ŷ	3��
	 3	TX2	 P1.4	�������裺200 + 10ŷ	�����������
	 4	TX1	 P1.3	�������裺0   + 10ŷ	6��
*/
void SetIRLedPower(uchar power){
	//��4��IO�ڶ����ó�ֻ��
	P1 |= 0x78;			//��P1�� 3 4 5 6 λ���óɸߣ��������ᴥ�������ź�
	P1M1 |= 0x78;		//��P1M1�� 3 4 5 6 λ���ó�1
	P1M0 &= 0x87;		//��P1M0�� 3 4 5 6 λ���ó�0

	switch(power)
	{
		//���ó�0��������������
		case 0:
			break;

		//������Ҫ����ĳһ��IO��Ϊǿ��
		case 1:
		case 2:
		case 3:
		case 4:
			Set0(P1M1, 7 - power);
			Set1(P1M0, 7 - power);
			break;

		//����������ԣ����óɲ�����
		default:
			break;
	}
}

uchar ReadIRLedPower(){
	//����P1M1
	uchar M1 = P1M1;
	
	//������λ���õ�P1M1������λ��ֵ�Ա����ж�
	//P1.0 P1.1 P1.2 		�Ǻ������ͷ
	//P1.3 P1.4 P1.5 P1.6	�Ǻ��ⷢ��ͷ
	//����������� 				�����
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
	1�����ö�ʱ����1us�ж�һ�Σ�������һ��ȫ�ֱ����������жϳ���֪������һ��
	2�����ͺ����źţ�����ʼ��ʱ
	3���յ��ź�ʱ��¼
	4���ź���ʧʱ��¼
	5�����жϳ������е�ȫ�ֱ�������ֵ�����������������Ϣ����Ϊͬ��ͨѶҲҪ�õ�TIME1
	6���ظ����ϲ����β���
*/
void TestIRRecvTimeLine(){
	uchar i, power, tmp;
	uchar start_H, start_L, end_H, end_L;
	uchar code TAB[3] = {0x01, 0x02, 0x04};
	
	//�ȴ�һ����Ϣ��ʱ��
	delayms(OneMsgTimeOfms);

	//����յ������ĸ����ʵ���Ϣ����������Ӧ����
	//�����˳�
	if(GetMsgCount() == 0){
		return;
	}

	//�õ���Ϣ
	tmp = GetMsg();

	//��֤��Ϣ,����ȷʱ�˳�
	if(!(tmp >= 0 && tmp <= 4)){
		return;
	}

	//�洢ԭ���Ĺ���
	power = ReadIRLedPower();

	//�����¹���
	SetIRLedPower(tmp);

	i = 0;
	//����Ϊÿ������ͷ����һ��
	while(i < 3){
	
		//��ʼ������
		start_H = start_L = end_H = end_L = 0;
	
		//�����ж�
		AUXR &= 0xBF;		//��ʱ��ʱ��12Tģʽ
		TMOD &= 0x0F;		//���ö�ʱ��ģʽ
		TMOD |= 0x10;		//���ö�ʱ��ģʽ
		TL1 = 0x00;			//���ö�ʱ��ֵ
		TH1 = 0x00;			//���ö�ʱ��ֵ
		TF1 = 0;			//���TF1��־
		
		//���ͼ������ڣ�Ĭ��Ϊ10�����ڣ����define.h
		Count38K = N_38K;									//����,ֱ��������� 
		IsWait = 0;
		TR0 = 1;
		TR1 = 1;		//��ʱ��1��ʼ��ʱ
	
		tmp = TAB[i];		//2016.4.20���·��ƶ���ѭ����
		while(1){
			//�ж��Ƿ�ʼ���յ��ź�
			//����δ���յ��ź� 
			//tmp = TAB[i];		   //2016.4.20ԭλ��
			if(start_L == 0 && start_H == 0){
				//һ�����յ��źžͼ�¼��start
				if((P1 & tmp) == 0){
					start_H = TH1;
					start_L = TL1;
					LED1 = LEDON;
				}
			}else if(end_L == 0 && end_H == 0){
				if((P1 & tmp) == tmp){		 //2016.4.20�޸�һ��bug,ԭ���� if(P1 & tmp == tmp){
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

		//�ָ���ʱ����ͬ��ͨѶ����
		AUXR |= 0x40;						//��ʱ��ʱ��1Tģʽ
		TMOD &= 0x0F;						//���ö�ʱ��ģʽ
		TMOD |= 0x10;						//���ö�ʱ��ģʽ
	    TH1 = COMMUNICATION_TH1;			//��ֵ�����ü�UseCommunication��
	    TL1 = COMMUNICATION_TL1;			//��ֵ�����ü�UseCommunication��
		ET1 = 1;							//��ʱ��1����λ

		//���ͼ���������õ�����
		SendMsg(start_H);
		SendMsg(start_L);
		SendMsg(end_H);
		SendMsg(end_L);
		i++;
		//��ΪҪ���ü�ʱ��������һ��Ҫ��SendMsg�����ʱ���ٿ�ʼ��һ�ֲ���
		while(TR1 == 1);
	}

	//���ͱ��ӻ�ID
	SendMsg(ID);
	while(TR1 == 1);

	//�ָ��ɹ���
	SetIRLedPower(power);
}

void XunHuanSendMsg(){
	uint i = 1;
	delayms(500);

	//��һ���˳�������
	while(i){
		//��˸LED
		LED1 = !LED1;
		SendMsg(i++);
	}
}