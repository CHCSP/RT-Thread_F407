
#include "main.h"

//#define LED0 PFout(9)   	//LED0
//#define LED1 PFout(10)   	//LED1

const u8 TEXT_Buffer[]={"Explorer STM32F4 SPI TEST"};
#define SIZE sizeof(TEXT_Buffer)

FATFS fs[2];//�߼����̹�����.	 
FIL fil;	  		//�ļ�1
u8 WriteChar[512] = {30,32,33,34,35,36,37,38,39,40};

//两个线程
static struct rt_thread test_thread;
static rt_uint8_t rt_test_thread_stack[256];
static struct rt_thread test_semaphore;
static rt_uint8_t rt_test_semaphore_stack[256];

//rt_sem_t sem_test;
//信号量
static struct rt_semaphore static_sem; 
rt_uint32_t semValue = 0;

static void test_thread_entry(void *parameter);
u8 rt_KEY1 = 0,rt_KEY2 = 0,rt_KEY0 = 0,rt_KEYUP = 0;

//定时器
rt_timer_t timer1;
struct rt_timer timer2;

//事件
struct rt_event static_event;
//rt_uint32_t mailx[128];

//邮箱
rt_uint8_t mailx[128];
rt_uint32_t mailxsendValue[2] = {333,200};
//rt_mailbox_t mb_handle;
struct rt_mailbox static_mb;


//消息队列
static struct rt_messagequeue mq;
static rt_uint8_t msg_pool[2048];


static void test_semaphore_entry(void *parameter)
{
    static rt_err_t result;
    rt_uint32_t e;
    rt_uint32_t *pdata;
    while(1)
    {
        result = rt_sem_take(&static_sem,RT_WAITING_FOREVER);           //��ȡ�ź�����δ��ȡ����һֱ�ȴ�
        if(result != RT_EOK)
        {
            printf("TIME1 TEST ERR!\r\n");
        }
        
        printf("Semphore TEST OK!\r\n");
        
      result = rt_event_recv(&static_event,0x05,RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER,&e);
      printf("EVENT number : %ld\r\n",e);
        
        
        if (rt_mb_recv(&static_mb,(rt_uint32_t*)&pdata, RT_WAITING_FOREVER) == RT_EOK)
        {
            
            printf("Mail OK : %ld\r\n",pdata[0]);
        
        }
        
        if (rt_mq_recv(&mq,&pdata[1],sizeof(pdata[1]),RT_WAITING_FOREVER) == RT_EOK)
        {
            
            printf("MQ OK : %ld\r\n",pdata[1]);
        
        }        
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        rt_thread_mdelay(200);
    }
}

static void test_thread_entry(void *parameter)
{
    u8 count = 0;
    
    char buf = 10;
    
    while(1)
    {
        //HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        //HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
        //HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
        //rt_kprintf("Kprintf test!!! \n");
        //ABC
        if((KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
        {
            count++;
            if(count == 5)
            {
                if(KEY0==0)
                {
                    rt_KEY0 = 0xff;
                    rt_event_send(&static_event,0x01);
                    
                    rt_sem_release(&static_sem);        //�ͷ��ź���
                    //rt_mb_init(&static_mb,"mailbox",mailx,sizeof(mailx)/4,RT_IPC_FLAG_FIFO);
                    rt_mb_send(&static_mb,(rt_uint32_t)&mailxsendValue[0]);
                    
                    
                    rt_mq_send(&mq,&buf,1);
                    buf++;
                }
                else if(KEY1==0)
                {
                    rt_KEY1 = 0xff;
                    rt_event_send(&static_event,0x04);
                }
                else if(KEY2==0)
                {
                    rt_KEY2 = 0xff;
                }
                else
                {
                    rt_KEYUP = 0xff;
                }
            }
            if(count > 5)
            {
                count = 10;
            }
        }
        else
        {
            count = 0;
        }
        
        
        rt_thread_mdelay(10);
    }    
}


static void rt_thread_idle_test1(void)
{
    if(rt_KEY0)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        rt_KEY0 = 0;
    }
       
}
static void rt_thread_timer1(void *parameter)
{
    if(rt_KEY1)
    {
        rt_KEY1 = 0;
        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin); 
    }
    //printf("TIME1 TEST!\r\n");
}


static void rt_thread_timer2(void *parameter)
{

    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin); 
    //printf("TIME1 TEST!\r\n");
}


//ͨ�����ڴ�ӡSD�������Ϣ
void show_sdcard_info(void)
{
	uint64_t CardCap;	//SD������
	HAL_SD_CardCIDTypeDef SDCard_CID;

	HAL_SD_GetCardCID(&SDCARD_Handler,&SDCard_CID);	//��ȡCID
	SD_GetCardInfo(&SDCardInfo);					//��ȡSD����Ϣ
	switch(SDCardInfo.CardType)
	{
		case CARD_SDSC:
		{
			if(SDCardInfo.CardVersion == CARD_V1_X)
				printf("Card Type:SDSC V1\r\n");
			else if(SDCardInfo.CardVersion == CARD_V2_X)
				printf("Card Type:SDSC V2\r\n");
		}
		break;
		case CARD_SDHC_SDXC:printf("Card Type:SDHC\r\n");break;
	}	
	CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//����SD������
  	printf("Card ManufacturerID:%d\r\n",SDCard_CID.ManufacturerID);					//������ID
 	printf("Card RCA:%d\r\n",SDCardInfo.RelCardAdd);								//����Ե�ַ
	printf("LogBlockNbr:%d \r\n",(u32)(SDCardInfo.LogBlockNbr));					//��ʾ�߼�������
	printf("LogBlockSize:%d \r\n",(u32)(SDCardInfo.LogBlockSize));					//��ʾ�߼����С
	printf("Card Capacity:%d MB\r\n",(u32)(CardCap>>20));							//��ʾ����
 	printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.BlockSize);						//��ʾ���С
}


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);        //BEEP
    //printf("ALARM A!\r\n");
}



void App_Main(void)
{
    u8 key = 0;
    u8 t; 
    u16 times=0;
    
 	u8 x=0;
	u8 lcd_id[12];				//���LCD ID�ַ���   
    
    
    
	u16 i=0;
	u8 datatemp[SIZE];
	u32 FLASH_SIZE;
    u16 id = 0;  
    
    
	u8 *buf;
	uint64_t CardCap;				//SD������    
    UINT len;
    
    
 	RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    u8 tbuf[40];   
    
    
    
    
    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
    
    
 	LCD_Init();           			//��ʼ��LCD FSMC�ӿ�
    
    
    W25QXX_Init();
	POINT_COLOR=RED;     			//������ɫ����ɫ
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣    
    
/*  LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"SPI TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2017/4/13");	 		
	LCD_ShowString(30,130,200,16,16,"KEY1:Write  KEY0:Read");	//��ʾ��ʾ��Ϣ
	while(1)
	{
		id = W25QXX_ReadID();
        printf("\r\n%d",id);
		if (id == W25Q128 || id == NM25Q128)
			break;
		LCD_ShowString(30,150,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,150,200,16,16,"Please Check!        ");
		delay_ms(500);
		HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
	}
	LCD_ShowString(30,150,200,16,16,"W25Q128 Ready!"); 
	FLASH_SIZE=32*1024*1024;	//FLASH ��СΪ32M�ֽ�
  	POINT_COLOR=BLUE;			//��������Ϊ��ɫ	    
    */
    
   	/*LCD_ShowString(30,50,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,70,200,16,16,"SD CARD TEST");	
	LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,110,200,16,16,"2017/4/18");     
	LCD_ShowString(30,130,200,16,16,"KEY0:Read Sector 0");	  */


	show_sdcard_info();	//��ӡSD�������Ϣ
 	POINT_COLOR=BLUE;	//��������Ϊ��ɫ 
	//���SD���ɹ� 											    
 
    
    //f_mount(&fs[0],"0:",1); //���� SD ��
    if(f_mount(&fs[0],"0:",1))
    {
        LCD_ShowString(30,150,200,16,16,"SD Card error    ");
    }
    else
    {
        LCD_ShowString(30,150,200,16,16,"SD Card OK    ");
        LCD_ShowString(30,170,200,16,16,"SD Card Size:     MB");
        CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//����SD������
        LCD_ShowNum(30+13*8,170,CardCap>>20,5,16);//��ʾSD������	

        
/*        f_open(&fil,"cyc20220528.txt",FA_CREATE_ALWAYS | FA_WRITE);      
        f_write(&fil,"FATFS TEST 0528\n",sizeof("FATFS TEST 0528\n"),&len);
        f_close(&fil);
 */       
    }
 

//    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);


    //线程初始化
   rt_thread_init(&test_thread,
                   "test_thread",
                   test_thread_entry,
                   RT_NULL,
                   &rt_test_thread_stack,
                   sizeof(rt_test_thread_stack),
                   30,
                   5); 
    
    //空闲钩子函数
    rt_thread_idle_sethook(rt_thread_idle_test1);                                   //���й��Ӻ���
//    rt_thread_idle_sethook(rt_thread_idle_test2); 
    //rt_thread_timer_test1
//    rt_timer_init(timer1,"timer_tes1",rt_thread_timer1,RT_NULL,300,3);

    //动态创建一个定时器
    timer1 = rt_timer_create("timer_tes1",rt_thread_timer1,RT_NULL,300,3);          //��̬��ʱ������
    if(rt_timer_start(timer1))
        printf("TIME1 TEST err!\r\n");          
      
    //静态创建定时器
    rt_timer_init(&timer2,"timer_tes2",rt_thread_timer2,RT_NULL,1000,3);            //��̬��ʱ������
    if(rt_timer_start(&timer2))
        printf("TIME2 TEST err!\r\n");          

    //创建邮箱
    rt_mb_init(&static_mb,"mailbox",mailx,sizeof(mailx)/4,RT_IPC_FLAG_FIFO);

    //创建消息队列
    rt_mq_init(&mq,"mqt",&msg_pool[0],1,sizeof(msg_pool),RT_IPC_FLAG_FIFO);

//    rt_timer_init(timer1,"timer_tes1",rt_thread_timer1,RT_NULL,1000,3);
//    if(rt_timer_start(timer1))
//        printf("TIME1 TEST err!\r\n");   
        //test_semaphore_entry
    //创建线程
    rt_thread_init(&test_semaphore,
                   "test_semaphore",
                   test_semaphore_entry,
                   RT_NULL,
                   &rt_test_semaphore_stack,
                   sizeof(rt_test_semaphore_stack),
                   27,
                   5);        
    //创建事件
    rt_event_init(&static_event,"event_test",RT_IPC_FLAG_FIFO);    
        
    //信号量
    rt_sem_init(&static_sem,"dsem",semValue,RT_IPC_FLAG_FIFO);          //�ź�����ʼ��
    //sem_test = rt_sem_create("dsem",semValue,RT_IPC_FLAG_FIFO);
    rt_thread_startup(&test_thread);                                    //��������
    rt_thread_startup(&test_semaphore);                                 //��������
    return ;
    
    while(1)
    {
//        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
//        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
//        HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);        //BEEP
//        printf("LEDtog\n\r");
//        delay_ms(500);
        
        
       key = KEY_Scan(1);
       
       
       
       
       
		t++;
		if((t%10)==0)	//ÿ100ms����һ����ʾ����
		{
            HAL_RTC_GetTime(&hrtc,&RTC_TimeStruct,RTC_FORMAT_BIN);
			sprintf((char*)tbuf,"Time:%02d:%02d:%02d",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds); 
			LCD_ShowString(30,50,210,16,16,tbuf);	
            HAL_RTC_GetDate(&hrtc,&RTC_DateStruct,RTC_FORMAT_BIN);
			sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date); 
			LCD_ShowString(30,70,210,16,16,tbuf);	
			sprintf((char*)tbuf,"Week:%d",RTC_DateStruct.WeekDay); 
			LCD_ShowString(30,90,210,16,16,tbuf);
		} 
		//if((t%20)==0)LED0=!LED0;	//ÿ200ms,��תһ��LED0 
        delay_ms(10);      
       
       
       
       
       
       
       
        
/*
 if(key==KEY1_PRES)//KEY1����,д��W25Q128
		{
			LCD_Fill(0,170,239,319,WHITE);//�������    
 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128....");
			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);		//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
			LCD_ShowString(30,170,200,16,16,"W25Q256 Write Finished!");	//��ʾ�������
		}
		if(key==KEY0_PRES)//KEY0����,��ȡ�ַ�������ʾ
		{
 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:   ");	//��ʾ�������
			LCD_ShowString(30,190,200,16,16,datatemp);					//��ʾ�������ַ���
		} 
		i++;
		delay_ms(10);
		if(i==20)
		{
			HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
			i=0;
		}       

*/        
       
       
       
       
       
       
       
       
       
       
       /*switch(key)
       {
        case WKUP_PRES: //���� LED0,LED1 �������
        {
            printf("LEDtog\n\r");
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
        }break;
        case KEY2_PRES: //���� LED0 ��ת
        {
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);      
        }break;
        case KEY1_PRES: //���� LED1 ��ת
        {
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);     
        }break;
        case KEY0_PRES: //ͬʱ���� LED0,LED1 ��ת
        {
            HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);     
        }break;
        default:break;
       }*/
        
        if(USART_RX_STA&0x8000)
        {
            /*
            len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
            printf("\r\n �����͵���ϢΪ:\r\n");
            for(t=0;t<len;t++)
            {
                USART1->DR=USART_RX_BUF[t];
                while((USART1->SR&0X40)==0);//�ȴ����ͽ���
            }
            printf("\r\n\r\n");//���뻻��
            
            
            if(USART_RX_BUF[0] == '1')
            {
                (*(u32(*)())func[0])(0,1);                
            }
            else if(USART_RX_BUF[0] == '2')
            {
                (*(u32(*)())func[0])(1,1);     
            }
             else if(USART_RX_BUF[0] == '3')
            {
                ((u32(*)())func[1])(0);   
            }
            else if(USART_RX_BUF[0] == '4')
            {
                ((u32(*)())func[1])(1); 
            }
            else if(USART_RX_BUF[0] == '5')
            {
                (*(u32(*)())func[2])(); 
            }
            else if(USART_RX_BUF[0] == '6')
            {
                (*(u32(*)())func[3])(); 
            }
            else if(USART_RX_BUF[0] == '7')
            {
                (*(u32(*)())func[4])(); 
            }
            else if(USART_RX_BUF[0] == '8')
            {
                (*(u32(*)())func[5])(); 
            }   
            else if(USART_RX_BUF[0] == '9')
            {
                (*(u32(*)())func[0])(0,0);   
            } */           

            USART_RX_STA=0;
        }else
        {
            times++;
            if(times%5000==0)
            { 
                printf("\r\nALIENTEK ̽���� STM32F407 ������ ����ʵ��\r\n");
                printf("����ԭ��@ALIENTEK\r\n\r\n\r\n");
            }
            if(times%200==0)printf("����������,�Իس�������\r\n");
            if(times%30==0)HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);//��˸ LED,��ʾϵͳ��������.
            delay_ms(10);
        }
        
        
        
    /*    
  switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break; 
			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR=RED;	  
		LCD_ShowString(30,40,210,24,24,"Explorer STM32F4");	
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//��ʾLCD ID	      					 
		LCD_ShowString(30,130,200,12,12,"2017/4/8");	      					 
		x++;
		if(x==12)x=0;
		//HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);	 
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
		delay_ms(1000);	      
        */
        
        
        
    }
}



































