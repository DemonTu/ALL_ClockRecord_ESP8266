#ifndef __USERDS1302DRIVER_H__
#define __USERDS1302DRIVER_H__

#define ds1302_sec_add			0x80		//�����ݵ�ַ
#define ds1302_min_add			0x82		//�����ݵ�ַ
#define ds1302_hr_add			0x84		//ʱ���ݵ�ַ
#define ds1302_date_add			0x86		//�����ݵ�ַ
#define ds1302_month_add		0x88		//�����ݵ�ַ
#define ds1302_day_add			0x8a		//�������ݵ�ַ
#define ds1302_year_add			0x8c		//�����ݵ�ַ
#define ds1302_control_add		0x8e		//�������ݵ�ַ
#define ds1302_charger_add		0x90 					 
#define ds1302_clkburst_add		0xbe


typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t data;
	uint8_t hour;
	uint8_t minute;
}TIME_STR;
/*------------------------------------------------
                DS1302��ʼ��
------------------------------------------------*/
extern void userDS12302_Init(void);


#endif
/**************************************************************************************************/

