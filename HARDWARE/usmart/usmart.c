#include "usmart.h"





void* func[10]=
{
    (void*)LED_Set,
    (void*)Beep_Ctr,
    (void*)LED0_ON,
    (void*)LED0_OFF,
    (void*)LED1_ON,
    (void*)LED1_OFF
};



void LED_Set(u8 num,u8 mode)
{
    if(num == 0)
    {
        if(mode)
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
    }
    else
    {
        if(mode)
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);        
    }
}

void Beep_Ctr(u8 mode)
{
    if(mode)
        //HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
}


void LED0_ON(void)
{
    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
}

void LED0_OFF(void)
{
    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
}

void LED1_ON(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);  
}

void LED1_OFF(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
}
















