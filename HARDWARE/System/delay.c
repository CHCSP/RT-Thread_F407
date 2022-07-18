#include "delay.h"

static u32 fac_us=0;							//us延时倍乘数


void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	u32 reload;
#endif
//    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
	fac_us=SYSCLK;						//不论是否使用OS,fac_us都需要使用
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	reload=SYSCLK;					    //每秒钟的计数次数 单位为K	   
	reload*=1000000/delay_ostickspersec;	//根据delay_ostickspersec设定溢出时间
											//reload为24位寄存器,最大值:16777216,在180M下,约合0.745s左右	
	fac_ms=1000/delay_ostickspersec;		//代表OS可以延时的最少单位	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/OS_TICKS_PER_SEC秒中断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //开启SYSTICK
#else
#endif
}





void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*fac_us; 						//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};
}

//延时nms
//nms:要延时的ms数
void delay_ms(u16 nms)
{
	u32 i;
	for(i=0;i<nms;i++) delay_us(1000);
}














