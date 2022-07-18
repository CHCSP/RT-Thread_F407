#include "usart.h"

u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���
u8 aRxBuffertemp = 0;
u8 aRxBuffertemplen = 0;

UART_HandleTypeDef UART1_Handler; //UART���


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//if(huart->Instance==USART1)//����Ǵ���1
    if(huart == &huart1)
	{
        
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else 
                {
                    USART_RX_STA|=0x8000;	//��������� 
                    
                    aRxBuffertemp = 1;
                    aRxBuffertemplen = (USART_RX_STA & 0x3FFF);
                    
                }
			}
			else //��û�յ�0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}

	}
}






#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
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
                while((USART1->SR&0X40)==0);    //ѭ������,ֱ���������   
                USART1->DR = (u8) a;     
            }
            //while (Reset == USART_GetStatus(USART_CH, UsartTxEmpty)){};  // Warit Tx data register empty //
            //USART_SendData(USART_CH,(*(str + i)));
            
                while((USART1->SR&0X40)==0);    //ѭ������,ֱ���������   
                
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
    
 
    //ch = aRxBuffer[0];    //��������
    return ch;
}




