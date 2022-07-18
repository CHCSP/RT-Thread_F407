#ifndef __USMART_H_
#define __USMART_H_
#include "main.h"




void LED_Set(u8 num,u8 mode);
void Beep_Ctr(u8 mode);
void LED0_ON(void);
void LED0_OFF(void);
void LED1_ON(void);
void LED1_OFF(void);


extern void* func[10];



void rt_hw_console_output(const char *str);
char rt_hw_console_getchar(void);











#endif



