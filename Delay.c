#include "stc12c5a60s2.h"
#include "Delay.h"
#include <intrins.h>

#ifdef USEDELAY
void delayms(unsigned int n)
{
	unsigned int ms;
	while(n--)
		for(ms = 0; ms < Const1ms; ms++);
}
/*
void delay500us(void)   //Îó²î 0us
{
    unsigned char a,b;
    for(b=111;b>0;b--)
        for(a=12;a>0;a--);
}

void delay1us(void)   //Îó²î 0us
{
    _nop_();  //if Keil,require use intrins.h
}

void delay2us(void)   //Îó²î 0us
{
    unsigned char a;
    for(a=3;a>0;a--);
}

void delay5us(void)   //Îó²î 0us
{
    unsigned char a;
    for(a=12;a>0;a--);
}
*/

void delay10us(void)   //Îó²î 0us
{
    unsigned char a;
    for(a=27;a>0;a--);
}

void delay100us(void)   //Îó²î 0us
{
    unsigned char a,b;
    for(b=66;b>0;b--)
        for(a=3;a>0;a--);
}


void delay200us(void)   //Îó²î 0us
{
    unsigned char a,b;
    for(b=6;b>0;b--)
        for(a=98;a>0;a--);
}

#endif