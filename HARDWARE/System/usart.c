#include "usart.h"

u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
u8 aRxBuffertemp = 0;
u8 aRxBuffertemplen = 0;

UART_HandleTypeDef UART1_Handler; //UART句柄


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//if(huart->Instance==USART1)//如果是串口1
    if(huart == &huart1)
	{
        
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else 
                {
                    USART_RX_STA|=0x8000;	//接收完成了 
                    
                    aRxBuffertemp = 1;
                    aRxBuffertemplen = (USART_RX_STA & 0x3FFF);
                    
                }
			}
			else //还没收到0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}

	}
}






#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 



void rt_hw_console_output(const char *str)
{
        rt_size_t i = 0, size = 0;
        char a = '\r';
        size = rt_strlen(str);
                
        for (i = 0; i < size; i++)
        {
            if (*(str + i) == '\n')
            {
                //while (Reset == USART_GetStatus(USART_CH, UsartTxEmpty)) {}; // Warit Tx data register empty //
                //USART_SendData(USART_CH,(uint16_t)a);
                while((USART1->SR&0X40)==0);    //循环发送,直到发送完毕   
                USART1->DR = (u8) a;     
            }
            //while (Reset == USART_GetStatus(USART_CH, UsartTxEmpty)){};  // Warit Tx data register empty //
            //USART_SendData(USART_CH,(*(str + i)));
            
                while((USART1->SR&0X40)==0);    //循环发送,直到发送完毕   
                
                USART1->DR = (u8) (*(str + i));     
        }
}


char rt_hw_console_getchar(void)
 {
    int ch = -1;
    static int len = 0,n = 0;;
    //u32 timeout;
    if (aRxBuffertemp)
    {
        if(aRxBuffertemp == 1)
        {
            aRxBuffertemp = 2;
            len = aRxBuffertemplen;
            n = 0;
        }
        if(aRxBuffertemp == 2)
        {
            ch = USART_RX_BUF[n++];
            if(n > len)
            {
                aRxBuffertemp = 0;
                USART_RX_STA = 0;
                ch = '\n';
            }
        }
    }
    else
    {
        //ch = '\n';
        rt_thread_mdelay(50);
    }
    
 
    //ch = aRxBuffer[0];    //接收数据
    return ch;
}




