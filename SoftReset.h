#include "stc12c5a60s2.h"

#ifndef SOFTRESET
#define SOFTRESET

#define		SelfDefineISPDownloadCommand		0x22

void SoftResetToISPMonitor();
void Init_UART_For_Download();
#endif

