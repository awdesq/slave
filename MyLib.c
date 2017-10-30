#include "stc12c5a60s2.h"
#include "MyLib.h"
#include <intrins.h>

#ifdef	USE_UART_FOR_AUTO_DOWNLOAD
//���³����ô���1��ʵ���Զ�����,�Զ��������в����ʾ͵�Ϊ14400bps,���Ƕ���12M�ľ���,�������������¼���


#ifdef USE_SOME_DELAY
//һЩ delay ����
void delayms(unsigned int n)
{
	unsigned int ms;
	while(n--)
		for(ms = 0; ms < Const1ms; ms++);
}

void delay1us(void)   //��� 0us
{
    _nop_();  //if Keil,require use intrins.h
}

void delay2us(void)   //��� 0us
{
    unsigned char a;
    for(a=3;a>0;a--);
}

void delay5us(void)   //��� 0us
{
    unsigned char a;
    for(a=12;a>0;a--);
}

void delay10us(void)   //��� 0us
{
    unsigned char a;
    for(a=27;a>0;a--);
}

void delay100us(void)   //��� 0us
{
    unsigned char a,b;
    for(b=66;b>0;b--)
        for(a=3;a>0;a--);
}

void delay200us(void)   //��� 0us
{
    unsigned char a,b;
    for(b=6;b>0;b--)
        for(a=98;a>0;a--);
}

void delay500us(void)   //��� 0us
{
    unsigned char a,b;
    for(b=111;b>0;b--)
        for(a=12;a>0;a--);
}

void delay1ms(void)   //��� 0us
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
sbit	StateIn = P3^2;							//ͨѶ���ն˶˿�,ע�⽫�˿���Ϊֻ��P1M1.n = 1 P1M0.n = 0
sbit	StateOut = P1^5; 						//Ҫע�⽫StateOut����Ϊǿ���� P1M1 = 0 P1M0 = 1
uchar	MsgBuffer[MsgBufferLen];				//����һ����������Ϣ���յĻ�����
uchar	*pStart, *pEnd;							//���ǻ��������׵�ַ��ĩ��ַ
uchar	*pMsg, *pNewMsg;						//һ��ָ��ָ����һ��Ҫ�������Ϣ��һ��ָ��ָ�����µ���Ϣ��ע����ϢӦ���ǡ������ȳ�FIFO 
uchar	Msg;									//���ڴ洢Ҫ���͵�����		
uchar	T1Count;								//���ڼ�¼���붨ʱ���жϵĴ���,���ⲿ�жϸ����声
bit		SendOrGet;								//ָʾ�Ƿ��ͻ��ǽ������ݣ�����Ǹ�Time1Interrupt���õ� 1 SendMsg 0 GetMsg

void Init_For_Communication(void)
{
	//��ʼ����ʱ��
	AUXR = AUXR | 0x40;  				// T1, 1T Mode
    TMOD = 0x10 | (TMOD & 0x0F);		//16λ��ʱ��
    TH1 = COMMUNICATION_TH1;			//��ֵ
    TL1 = COMMUNICATION_TL1;			//��ֵ
    ET1 = 1;							//��ʱ��1����λ

	//��ʼ����ص�ַ
	pStart = pMsg = pNewMsg = &MsgBuffer;		//��û��������׵�ַ,ĩ��ַ,����Ϣ�洢ָ��,Ҫ�������Ϣָ��
	pEnd = &MsgBuffer + MsgBufferLen - 1;

	//�ͷ�����
	StateOut = 1;								//���1ʱ,�����ܲ���ͨ,����Ϊ��

	//����IO��ģʽ
	Set0(P1M1, 5);								//����StateOutΪǿ����
	Set1(P1M0, 5);
	Set1(P3M1, 2);								//����StateInΪֻ��
	Set0(P3M0, 2);
	
	PX0 = 1;					//���ⲿ�ж���Ϊ������ȼ�,��ʱ���жϱ�����һ��
	IT0 = 1; 					//�½��ش����ж�
   	IE0 = 0;					//���־λ
	EX0 = 1;					//���ⲿ�ж�0
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
	//���pMsg == pNewMsg ˵��û����Ϣ��ȡ,����NULLMsg
	if(pMsg == pNewMsg)
		return NULLMsg;

	if(pMsg == pEnd)		//��������Ϣλ�ڻ�������β��,��ָ�뽫�Ƶ��ײ�
	{
		pMsg = pStart;
		return *pEnd;
	}
	else					//�������β��,����Ϣ��1������
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
	while(TR1  == 1);				//���TR1 == 1,˵��Ҫô�ڷ�������,Ҫô���ڽ�������,�ȴ������
   	delay100us();					//����Ҫ��Ҫ��ʱ,��ʵ�����,���Ǽ��˱Ƚϰ�ȫ,����Ӧ����ʱ���㹻�����߻ָ����ߵ�ƽ
	EX0 = 0;						//Ϊ�������ж�,���ⲿ�жϹر��ٷ���Ϣ
	T1Count = 0;					//����Time1���жϴ���
	SendOrGet = SendMsgFlag;		//˵���ڶ�ʱ���ж����÷������ݵ���һ�׳���
	Msg = myMsg;					//�洢Ҫ���͵����ݵ����ñ���
	StateOut = 0;					//�������ߵ�ƽ,���������Ϣ�ⲿ�ж�,����ʼ��ʱ�䷢����Ϣ
	TR1 = 1;						//������ʱ��
	return SUCCESS;		
}
/*
ͨѶЭ��:
���߽ṹ:		Out��������10K��������,�ӻ�����������������.
���͵�����:		1bitǰ�� + 8bit����,�Ӹ�λ������λ,ÿλ1ms
��ӳ��ϵ:		�����ϱ�����,��������0,�ߵ�ƽ����1,1bitǰ��Ӧʹ��������1ms
pMsg��pNewMsgָ��ͬһλ�� ��ʾû����Ϣ
pMsgָ��Ҫ�������Ϣ
pNewMsgָ��Ҫ�洢����Ϣ��λ��

*/
void Timer1Interrupt(void) interrupt 3
{
    TH1 = COMMUNICATION_TH1;
    TL1 = COMMUNICATION_TL1;
	T1Count++;						//�ܹ�T1Count����֪���ǵڼ��ν����ж�
	if(SendOrGet == GetMsgFlag)		//��������GetMsg,���Tʱ�䷢һλ����,���жϼ��ΪT/2,��3T/2 5T/2......17T/2���в���
	{
	
		//>2˵�����˵�һ��������
		//%2=1˵����������T/2,ǡ��λ��һλ���ݵ����м�
		//=17��T/2ʱ�����һλ����
		if(((T1Count % 2) == 1) && (T1Count > 2) && (T1Count <= 17))	
		{
			*pNewMsg = (*pNewMsg & 0xFE) | StateIn;	//��0λ����StateInһ��������
			if(T1Count < 17)		//����17ʱ��һλ��Ҫ����,�ͷ��ڵ�0λ,���ƻ������λ��ʧ
				*pNewMsg <<= 1;
		}
		else if(T1Count == 18)		//��18���ж����ƶ�ָ��
		{
			if(*pNewMsg != NULLMsg)	//���յ�����Ϣ�ǲ���NULLMsg,�����,���Լ�����һ�䲻�ƶ���Ϣ��ջָ��
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
			IE0 = 0;//���жϱ�־λ
			EX0 = 1;//���´�int0�ж�
			TR1 = 0;//ֹͣ��ʱ��
		}		
	}
	else							//��������SendMsg 						   
	{
		if(T1Count == 18)		//����18ʱ��ʾ�Ѿ��������
		{
			StateOut = 1;		//�ͷ�����
			IE0 = 0;			//���ⲿ�жϱ�־ 													 
			EX0 = 1;			//���жϼ���
			TR1 = 0;			//�ض�ʱ��,��������
		}
		else if(T1Count % 2 == 0)	//��2�� T/2�ͷ���һλ	
		{
			StateOut = Msg & 0x80;	
			Msg <<= 1;	
		}		
	}
}

void Int0_Interrupt(void) interrupt 0
{
	EX0 = 0;//�ر�int0,��һ������жϷ������Ҫ��������Ȩ������ʱ���������϶�����,�����ڶ������ڼ�Ҫ�ر��ⲿ�ж�
	SendOrGet = GetMsgFlag;//��ʾTime1���ڽ�������
	T1Count = 0;//����Ǹ�Time1�õ�,��Time1�жϷ������֪����int0������Ȩ����������ʱ�ǵڼ��ν���Time1�жϷ������
	TR1 = 1;//������ʱ����ʼ��
}
#endif