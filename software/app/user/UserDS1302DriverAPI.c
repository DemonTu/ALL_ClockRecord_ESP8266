#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

#include "gpio.h"
#include "user_interface.h"

/* BSP library */
#include "driver/i2c_master.h"
#include "UserDS1302DriverAPI.h"


//=======================================================
#define DS1302_RST_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define DS1302_RST_IO_NUM     13
#define DS1302_RST_IO_FUNC    FUNC_GPIO13

//=======================================
LOCAL os_timer_t DSPro_timer;


/* 双向数据 */
#define IO_CLR()  GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO), 0)	//电平置低
#define IO_SET()  GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO), 1)//电平置高
#define IO_R()	  GPIO_INPUT_GET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO)) //电平读取


/* 时钟信号 */
#define SCK_CLR()	GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO), 0)//时钟信号
#define SCK_SET()	GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO), 1)//电平置高

/* 复位脚 */
#define RST_CLR()		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_RST_IO_NUM),0)//电平置低
#define RST_SET()		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_RST_IO_NUM),1)//电平置高


unsigned char time_buf1[8] = {20,10,6,5,12,55,00,6};  //空年月日时分秒周
unsigned char time_buf[8] ;                           //空年月日时分秒周
//===============================================
LOCAL ICACHE_FLASH_ATTR
I2C_Gpio_Init(void)
{
	ETS_GPIO_INTR_DISABLE() ;
//    ETS_INTR_LOCK();

    PIN_FUNC_SELECT(I2C_MASTER_SDA_MUX, I2C_MASTER_SDA_FUNC);
    PIN_FUNC_SELECT(I2C_MASTER_SCL_MUX, I2C_MASTER_SCL_FUNC);

    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SDA_GPIO));
    GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO)), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE)); //open drain;
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << I2C_MASTER_SCL_GPIO));

    ETS_GPIO_INTR_ENABLE() ;
}

/*------------------------------------------------
           向DS1302写入一字节数据
------------------------------------------------*/
LOCAL bool ICACHE_FLASH_ATTR
Ds1302_Write_Byte(unsigned char addr, unsigned char d)
{
	unsigned char i;
	RST_SET();	
	
	/* 写入目标地址：addr */
	addr = addr & 0xFE;     //最低位置零
	for (i = 0; i < 8; i ++) 
	{ 
		if (addr & 0x01) 
		{
			IO_SET();
		}
		else 
		{
			IO_CLR();
		}
		os_delay_us(5);
		SCK_SET();
		os_delay_us(8);
		SCK_CLR();
		addr = addr >> 1;
	}
	
	/* 写入数据：d */
	for (i = 0; i < 8; i ++) 
	{
		if (d & 0x01) 
		{
			IO_SET();
		}
		else 
		{
			IO_CLR();
		}
		os_delay_us(5);
		SCK_SET();
		os_delay_us(8);
		SCK_CLR();
		d = d >> 1;
	}
	RST_CLR();					//停止DS1302总线
}

/*------------------------------------------------
           从DS1302读出一字节数据
------------------------------------------------*/

unsigned char Ds1302_Read_Byte(unsigned char addr) 
{

	unsigned char i;
	unsigned char temp;
	RST_SET();	
	os_delay_us(3);
	/* 写入目标地址：addr */
	addr = addr | 0x01;//最低位置高
	for (i = 0; i < 8; i ++) 
	    {
	     
		if (addr & 0x01) 
		{
			IO_SET();
		}
		else 
		{
			IO_CLR();
		}
		os_delay_us(5);
		SCK_SET();
		os_delay_us(8);
		SCK_CLR();
		addr = addr >> 1;
	}
	os_delay_us(5);
	/* 输出数据：temp */
	for (i = 0; i < 8; i ++) 
	{
		temp = temp >> 1;
		if (IO_R()) 
		{
			temp |= 0x80;
		}
		else 
		{
			temp &= 0x7F;
		}
		os_delay_us(5);
		SCK_SET();
		os_delay_us(8);
		SCK_CLR();
	}
	
	RST_CLR();	//停止DS1302总线
	return temp;
}

/*------------------------------------------------
           向DS1302写入时钟数据
------------------------------------------------*/
void Ds1302_Write_Time(void) 
{
     
    unsigned char i,tmp;
	for(i=0;i<8;i++)
	{                  //BCD处理
		tmp=time_buf1[i]/10;
		time_buf[i]=time_buf1[i]%10;
		time_buf[i]=time_buf[i]+tmp*16;
	}
	Ds1302_Write_Byte(ds1302_control_add,0x00);			//关闭写保护 
	Ds1302_Write_Byte(ds1302_sec_add,0x80);				//暂停
	
	/* 涓流充电 */
	//Ds1302_Write_Byte(ds1302_charger_add,0xa9);			 
	Ds1302_Write_Byte(ds1302_year_add,time_buf[1]);		//年 
	Ds1302_Write_Byte(ds1302_month_add,time_buf[2]);	//月 
	Ds1302_Write_Byte(ds1302_date_add,time_buf[3]);		//日 
	//Ds1302_Write_Byte(ds1302_day_add,time_buf[7]);		//周 
	Ds1302_Write_Byte(ds1302_hr_add,time_buf[4]);		//时 
	Ds1302_Write_Byte(ds1302_min_add,time_buf[5]);		//分
	Ds1302_Write_Byte(ds1302_sec_add,time_buf[6]);		//秒
	//Ds1302_Write_Byte(ds1302_day_add,time_buf[7]);		//周 

	Ds1302_Write_Byte(ds1302_control_add,0x80);			//打开写保护 
}
void userDS1302WriteTime(TIME_STR *time)
{
	os_memcpy(&time_buf1[1], (uint8_t *)time, sizeof(TIME_STR));
	time_buf[sizeof(TIME_STR)+1] = 0;
	Ds1302_Write_Time();	
}
/*------------------------------------------------
           从DS1302读出时钟数据
------------------------------------------------*/
void Ds1302_Read_Time(void)  
{ 
   	unsigned char i,tmp;
	
	time_buf[7]=Ds1302_Read_Byte(ds1302_day_add);		 //周 
	time_buf[1]=Ds1302_Read_Byte(ds1302_year_add);		 //年
	time_buf[2]=Ds1302_Read_Byte(ds1302_month_add);		 //月 
	time_buf[3]=Ds1302_Read_Byte(ds1302_date_add);		 //日 
	time_buf[4]=Ds1302_Read_Byte(ds1302_hr_add);		 //时 
	time_buf[5]=Ds1302_Read_Byte(ds1302_min_add);		 //分 
	time_buf[6]=(Ds1302_Read_Byte(ds1302_sec_add))&0x7F;//秒 
	time_buf[7]=Ds1302_Read_Byte(ds1302_day_add);		 //周 

	for(i=0;i<8;i++)
	{           //BCD处理
	//	os_printf("st=%x", time_buf[i]);
		tmp=time_buf[i]/16;
		time_buf1[i]=time_buf[i]%16;
		time_buf1[i]=time_buf1[i]+tmp*10;
	}
	//os_printf("\r\n");
}
void userDS1302ReadTime(TIME_STR *time)
{
	Ds1302_Read_Time();
	os_memcpy((uint8_t *)time, &time_buf1[1], sizeof(TIME_STR));
}
/*------------------------------------------------
                DS1302初始化
------------------------------------------------*/
void Ds1302_Init(void)
{
	
	RST_CLR();			//RST脚置低
	SCK_CLR();
    Ds1302_Write_Byte(ds1302_sec_add,0x00);
}

void ICACHE_FLASH_ATTR
UserDS1302Process(uint8_t flag)
{
	TIME_STR timeTemp;
	//Ds1302_Read_Time();
	userDS1302ReadTime(&timeTemp);
	os_printf("20%02d/%d/%d  %02d:%02d:%02d\r\n", timeTemp.year, timeTemp.month, timeTemp.data, timeTemp.hour, timeTemp.minute, time_buf1[6]);
	
}

/******************************************************************************
 * FunctionName : userDS12302_Init
 * Description  : 初始化 DS1302时钟IC
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
userDS12302_Init(void)
{
	// RST GPIO  init
	PIN_PULLUP_DIS(DS1302_RST_IO_MUX);
	PIN_FUNC_SELECT(DS1302_RST_IO_MUX, DS1302_RST_IO_FUNC);
	
	/* I2C 初始化 */
	I2C_Gpio_Init();
	
	/* DS1302 寄存器初始化 */
	Ds1302_Init();

	os_printf("DS1302 init ok\r\n");
	
/*定时扫描 rfid*/
	os_timer_disarm(&DSPro_timer);
	os_timer_setfn(&DSPro_timer, (os_timer_func_t *)UserDS1302Process, 1);
	os_timer_arm(&DSPro_timer, 1000, 1);
}

