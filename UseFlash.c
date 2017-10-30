#include "stc12c5a60s2.h"
#include "UseFlash.h"
#include <intrins.h>
#ifdef USEFLASH

//Ϊ��ȫ��Ҫ�����־λ
void IAP_Idle(void)
{
	Set0(IAP_CONTR, 7);				//��ֹIAP����
	IAP_CMD = IAP_IDLE;				//�������
	
}

//���һ������
void IAP_Erase_Sector(uint addr)
{
	IAP_CONTR = WAITTIME12M;		//����CPU�ȴ�ʱ��,���������
	Set1(IAP_CONTR, 7);				//����IAP����
	IAP_CMD = IAP_SECTOR_ERASE;		//������������Ĵ���
	IAP_ADDRL = addr;			   	//16λ��ַ�����ַ�Ĵ���
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;			   	//�ĵ��涨��������5A����A5,Ȼ��ʼ����
	IAP_TRIG = 0xA5;				//CPU�ڴ�֮���ֹͣ������ֱ��IAP����CONTR�ȴ���ʱ�����
	_nop_();						//���ĵ��ȴ�һ������
	IAP_Idle();						//Ϊ��ȫ��Ҫ�����־λ
}

//���һ���ֽ�
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

//��ȡһ���ֽ�
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