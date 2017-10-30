#include "stc12c5a60s2.h"

#ifndef USEFLASH
#define USEFLASH

#define		IAP_IDLE			0x00	/*��������*/
#define 	IAP_READ			0x01	/*  �ֽڶ�Ӧ�ó����������ݴ洢��   */
#define 	IAP_PROGRAM			0x02	/*  �ֽڱ��Ӧ�ó����������ݴ洢�� */
#define 	IAP_SECTOR_ERASE	0x03	/*  ��������Ӧ�ó����������ݴ洢�� */
#define		WAITTIME30M			0
#define		WAITTIME24M			1
#define		WAITTIME20M			2
#define		WAITTIME12M			3
#define		WAITTIME6M			4
#define		WAITTIME3M			5
#define		WAITTIME2M			6
#define		WAITTIME1M			7

void IAP_Idle(void);
void IAP_Erase_Sector(uint addr);
void IAP_Program_Byte(uint addr, uchar dataByte);
uchar IAP_Read_Byte(uint addr);

#endif