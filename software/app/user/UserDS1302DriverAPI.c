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


/* ˫������ */
#define IO_CLR()  GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO), 0)	//��ƽ�õ�
#define IO_SET()  GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO), 1)//��ƽ�ø�
#define IO_R()	  GPIO_INPUT_GET(GPIO_ID_PIN(I2C_MASTER_SDA_GPIO)) //��ƽ��ȡ


/* ʱ���ź� */
#define SCK_CLR()	GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO), 0)//ʱ���ź�
#define SCK_SET()	GPIO_OUTPUT_SET(GPIO_ID_PIN(I2C_MASTER_SCL_GPIO), 1)//��ƽ�ø�

/* ��λ�� */
#define RST_CLR()		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_RST_IO_NUM),0)//��ƽ�õ�
#define RST_SET()		GPIO_OUTPUT_SET(GPIO_ID_PIN(DS1302_RST_IO_NUM),1)//��ƽ�ø�


unsigned char time_buf1[8] = {20,10,6,5,12,55,00,6};  //��������ʱ������
unsigned char time_buf[8] ;                           //��������ʱ������
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
           ��DS1302д��һ�ֽ�����
------------------------------------------------*/
LOCAL bool ICACHE_FLASH_ATTR
Ds1302_Write_Byte(unsigned char addr, unsigned char d)
{
	unsigned char i;
	RST_SET();	
	
	/* д��Ŀ���ַ��addr */
	addr = addr & 0xFE;     //���λ����
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
	
	/* д�����ݣ�d */
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
	RST_CLR();					//ֹͣDS1302����
}

/*------------------------------------------------
           ��DS1302����һ�ֽ�����
------------------------------------------------*/

unsigned char Ds1302_Read_Byte(unsigned char addr) 
{

	unsigned char i;
	unsigned char temp;
	RST_SET();	
	os_delay_us(3);
	/* д��Ŀ���ַ��addr */
	addr = addr | 0x01;//���λ�ø�
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
	/* ������ݣ�temp */
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
	
	RST_CLR();	//ֹͣDS1302����
	return temp;
}

/*------------------------------------------------
           ��DS1302д��ʱ������
------------------------------------------------*/
void Ds1302_Write_Time(void) 
{
     
    unsigned char i,tmp;
	for(i=0;i<8;i++)
	{                  //BCD����
		tmp=time_buf1[i]/10;
		time_buf[i]=time_buf1[i]%10;
		time_buf[i]=time_buf[i]+tmp*16;
	}
	Ds1302_Write_Byte(ds1302_control_add,0x00);			//�ر�д���� 
	Ds1302_Write_Byte(ds1302_sec_add,0x80);				//��ͣ
	
	/* ������ */
	//Ds1302_Write_Byte(ds1302_charger_add,0xa9);			 
	Ds1302_Write_Byte(ds1302_year_add,time_buf[1]);		//�� 
	Ds1302_Write_Byte(ds1302_month_add,time_buf[2]);	//�� 
	Ds1302_Write_Byte(ds1302_date_add,time_buf[3]);		//�� 
	//Ds1302_Write_Byte(ds1302_day_add,time_buf[7]);		//�� 
	Ds1302_Write_Byte(ds1302_hr_add,time_buf[4]);		//ʱ 
	Ds1302_Write_Byte(ds1302_min_add,time_buf[5]);		//��
	Ds1302_Write_Byte(ds1302_sec_add,time_buf[6]);		//��
	//Ds1302_Write_Byte(ds1302_day_add,time_buf[7]);		//�� 

	Ds1302_Write_Byte(ds1302_control_add,0x80);			//��д���� 
}
void userDS1302WriteTime(TIME_STR *time)
{
	os_memcpy(&time_buf1[1], (uint8_t *)time, sizeof(TIME_STR));
	time_buf[sizeof(TIME_STR)+1] = 0;
	Ds1302_Write_Time();	
}
/*------------------------------------------------
           ��DS1302����ʱ������
------------------------------------------------*/
void Ds1302_Read_Time(void)  
{ 
   	unsigned char i,tmp;
	
	time_buf[7]=Ds1302_Read_Byte(ds1302_day_add);		 //�� 
	time_buf[1]=Ds1302_Read_Byte(ds1302_year_add);		 //��
	time_buf[2]=Ds1302_Read_Byte(ds1302_month_add);		 //�� 
	time_buf[3]=Ds1302_Read_Byte(ds1302_date_add);		 //�� 
	time_buf[4]=Ds1302_Read_Byte(ds1302_hr_add);		 //ʱ 
	time_buf[5]=Ds1302_Read_Byte(ds1302_min_add);		 //�� 
	time_buf[6]=(Ds1302_Read_Byte(ds1302_sec_add))&0x7F;//�� 
	time_buf[7]=Ds1302_Read_Byte(ds1302_day_add);		 //�� 

	for(i=0;i<8;i++)
	{           //BCD����
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
                DS1302��ʼ��
------------------------------------------------*/
void Ds1302_Init(void)
{
	
	RST_CLR();			//RST���õ�
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
 * Description  : ��ʼ�� DS1302ʱ��IC
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
userDS12302_Init(void)
{
	// RST GPIO  init
	PIN_PULLUP_DIS(DS1302_RST_IO_MUX);
	PIN_FUNC_SELECT(DS1302_RST_IO_MUX, DS1302_RST_IO_FUNC);
	
	/* I2C ��ʼ�� */
	I2C_Gpio_Init();
	
	/* DS1302 �Ĵ�����ʼ�� */
	Ds1302_Init();

	os_printf("DS1302 init ok\r\n");
	
/*��ʱɨ�� rfid*/
	os_timer_disarm(&DSPro_timer);
	os_timer_setfn(&DSPro_timer, (os_timer_func_t *)UserDS1302Process, 1);
	os_timer_arm(&DSPro_timer, 1000, 1);
}

