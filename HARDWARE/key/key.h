#ifndef __KEY_H_
#define __KEY_H_
#include "main.h"

#define KEY0_PRES 1
#define KEY1_PRES 2
#define KEY2_PRES 3
#define WKUP_PRES 4



#define KEY0 HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin) //KEY0 按键 PE4
#define KEY1 HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin) //KEY1 按键 PE3
#define KEY2 HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin) //KEY2 按键 PE2
#define WK_UP HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin) //WKUP 按键 PA0



u8 KEY_Scan(u8 mode);

















#endif











