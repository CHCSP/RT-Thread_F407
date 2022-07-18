#include "main.h"




SPI_HandleTypeDef SPI1_Handler;  //SPI1句柄
u16 W25QXX_TYPE=W25Q256;	//默认是W25Q256

//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{
    SPI1_Handler.Instance=SPI1;                         //SPI1
    SPI1_Handler.Init.Mode=SPI_MODE_MASTER;             //设置SPI工作模式，设置为主模式
    SPI1_Handler.Init.Direction=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    SPI1_Handler.Init.DataSize=SPI_DATASIZE_8BIT;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI1_Handler.Init.CLKPolarity=SPI_POLARITY_HIGH;    //串行同步时钟的空闲状态为高电平
    SPI1_Handler.Init.CLKPhase=SPI_PHASE_2EDGE;         //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI1_Handler.Init.NSS=SPI_NSS_SOFT;                 //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI1_Handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_256;//定义波特率预分频的值:波特率预分频值为256
    SPI1_Handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI1_Handler.Init.TIMode=SPI_TIMODE_DISABLE;        //关闭TI模式
    SPI1_Handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    SPI1_Handler.Init.CRCPolynomial=7;                  //CRC值计算的多项式
    HAL_SPI_Init(&SPI1_Handler);//初始化
    
    __HAL_SPI_ENABLE(&SPI1_Handler);                    //使能SPI1
	
    SPI1_ReadWriteByte(0Xff);                           //启动传输
}

//SPI5底层驱动，时钟使能，引脚配置
//此函数会被HAL_SPI_Init()调用
//hspi:SPI句柄
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();       //使能GPIOB时钟
    __HAL_RCC_SPI1_CLK_ENABLE();        //使能SPI1时钟
    
    //PB3,4,5
    GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;             //快速            
    GPIO_Initure.Alternate=GPIO_AF5_SPI1;           //复用为SPI1
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

//SPI速度设置函数
//SPI速度=fAPB1/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1时钟一般为42Mhz：
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&SPI1_Handler);            //关闭SPI
    SPI1_Handler.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    SPI1_Handler.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&SPI1_Handler);             //使能SPI
    
}

//SPI1 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 Rxdata;
    HAL_SPI_TransmitReceive(&SPI1_Handler,&TxData,&Rxdata,1, 1000);       
 	return Rxdata;          		    //返回收到的数据		
}





//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q256
//容量为32M字节,共有512个Block,8192个Sector 
													 
//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{ 
    u8 temp;
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();           //使能GPIOB时钟
    
    //PB14
    GPIO_Initure.Pin=GPIO_PIN_14;            //PB14
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //快速         
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化
    
	//W25QXX_CS=1;			                //SPI FLASH不选中
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);
	SPI1_Init();		   			        //初始化SPI
	SPI1_SetSpeed(SPI_BAUDRATEPRESCALER_4); //设置为21M时钟,高速模式
	W25QXX_TYPE=W25QXX_ReadID();	        //读取FLASH ID.
    if(W25QXX_TYPE==W25Q256)                //SPI FLASH为W25Q256
    {
        temp=W25QXX_ReadSR(3);              //读取状态寄存器3，判断地址模式
        if((temp&0X01)==0)			        //如果不是4字节地址模式,则进入4字节地址模式
		{
			//W25QXX_CS=0; 			        //选中
            HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
			SPI1_ReadWriteByte(W25X_Enable4ByteAddr);//发送进入4字节地址模式指令   
			//W25QXX_CS=1;       		        //取消片选   
            HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);
		}
    }
}  

//读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
//状态寄存器1：
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
//状态寄存器2：
//BIT7  6   5   4   3   2   1   0
//SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
//状态寄存器3：
//BIT7      6    5    4   3   2   1   0
//HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
//regno:状态寄存器号，范:1~3
//返回值:状态寄存器值
u8 W25QXX_ReadSR(u8 regno)   
{  
	u8 byte=0,command=0; 
    switch(regno)
    {
        case 1:
            command=W25X_ReadStatusReg1;    //读状态寄存器1指令
            break;
        case 2:
            command=W25X_ReadStatusReg2;    //读状态寄存器2指令
            break;
        case 3:
            command=W25X_ReadStatusReg3;    //读状态寄存器3指令
            break;
        default:
            command=W25X_ReadStatusReg1;    
            break;
    }    
	///W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
	SPI1_ReadWriteByte(command);            //发送读取状态寄存器命令    
	byte=SPI1_ReadWriteByte(0Xff);          //读取一个字节  
	//W25QXX_CS=1;                            //取消片选     
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);
	return byte;   
} 
//写W25QXX状态寄存器
void W25QXX_Write_SR(u8 regno,u8 sr)   
{   
    u8 command=0;
    switch(regno)
    {
        case 1:
            command=W25X_WriteStatusReg1;    //写状态寄存器1指令
            break;
        case 2:
            command=W25X_WriteStatusReg2;    //写状态寄存器2指令
            break;
        case 3:
            command=W25X_WriteStatusReg3;    //写状态寄存器3指令
            break;
        default:
            command=W25X_WriteStatusReg1;    
            break;
    }   
	//W25QXX_CS=0;                            //使能器件  
        HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
	SPI1_ReadWriteByte(command);            //发送写取状态寄存器命令    
	SPI1_ReadWriteByte(sr);                 //写入一个字节  
	//W25QXX_CS=1;                            //取消片选  
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
}   
//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
	//W25QXX_CS=0;                            //使能器件  
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);    
    SPI1_ReadWriteByte(W25X_WriteEnable);   //发送写使能  
	//W25QXX_CS=1;                            //取消片选     
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
} 
//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
	//W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_WriteDisable);  //发送写禁止指令    
	//W25QXX_CS=1;                            //取消片选    
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
} 

//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
//0XEF18,表示芯片型号为W25Q256
u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	//W25QXX_CS=0;			
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);    
	SPI1_ReadWriteByte(0x90);//发送读取ID命令	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	 			   
	Temp|=SPI1_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI1_ReadWriteByte(0xFF);	 
//	W25QXX_CS=1;				   
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET); 
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	//W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_ReadData);      //发送读取命令  
    if(W25QXX_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI1_ReadWriteByte((u8)((ReadAddr)>>24));    
    }
    SPI1_ReadWriteByte((u8)((ReadAddr)>>16));   //发送24bit地址    
    SPI1_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI1_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI1_ReadWriteByte(0XFF);    //循环读数  
    }
	//W25QXX_CS=1;  	
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    W25QXX_Write_Enable();                  //SET WEL 
	//W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_PageProgram);   //发送写页命令   
    if(W25QXX_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI1_ReadWriteByte((u8)((WriteAddr)>>24)); 
    }
    SPI1_ReadWriteByte((u8)((WriteAddr)>>16)); //发送24bit地址    
    SPI1_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI1_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//循环写数  
	//W25QXX_CS=1;                            //取消片选 
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);
	W25QXX_Wait_Busy();					   //等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
   	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 
}
//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                  //SET WEL 
    W25QXX_Wait_Busy();   
  	//W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_ChipErase);        //发送片擦除命令  
	//W25QXX_CS=1;                            //取消片选     
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
	W25QXX_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个扇区的最少时间:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
	//监视falsh擦除情况,测试用   
 	//printf("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL 	 
    W25QXX_Wait_Busy();   
  	//W25QXX_CS=0;                            //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_SectorErase);   //发送扇区擦除指令 
    if(W25QXX_TYPE==W25Q256)                //如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        SPI1_ReadWriteByte((u8)((Dst_Addr)>>24)); 
    }
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((u8)Dst_Addr);  
	//W25QXX_CS=1;                            //取消片选    
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
    W25QXX_Wait_Busy();   				    //等待擦除完成
}  
//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR(1)&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	//W25QXX_CS=0;                            //使能器件  
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);    
    SPI1_ReadWriteByte(W25X_PowerDown);     //发送掉电命令  
	//W25QXX_CS=1;                            //取消片选    
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
    delay_us(3);                            //等待TPD  
}   
//唤醒
void W25QXX_WAKEUP(void)   
{  
  	//W25QXX_CS=0;                                //使能器件   
    HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_RESET);
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);  //  send W25X_PowerDown command 0xAB    
	//W25QXX_CS=1;                                //取消片选  
HAL_GPIO_WritePin(F_CS_GPIO_Port,F_CS_Pin,GPIO_PIN_SET);    
    delay_us(3);                                //等待TRES1
    
}   












