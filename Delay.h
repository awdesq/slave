#include "stc12c5a60s2.h"
#include <intrins.h>

#ifndef USEDELAY
#define USEDELAY

#define Const1ms	595

void delayms(unsigned int n);
void delay1us(void);   
void delay2us(void);   
void delay5us(void);   
void delay10us(void);   
void delay100us(void);   
void delay200us(void);  
void delay500us(void);   
   
#endif
