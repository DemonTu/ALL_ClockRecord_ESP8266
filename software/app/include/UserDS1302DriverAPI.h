#ifndef __USERDS1302DRIVER_H__
#define __USERDS1302DRIVER_H__

#define ds1302_sec_add			0x80		//秒数据地址
#define ds1302_min_add			0x82		//分数据地址
#define ds1302_hr_add			0x84		//时数据地址
#define ds1302_date_add			0x86		//日数据地址
#define ds1302_month_add		0x88		//月数据地址
#define ds1302_day_add			0x8a		//星期数据地址
#define ds1302_year_add			0x8c		//年数据地址
#define ds1302_control_add		0x8e		//控制数据地址
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
                DS1302初始化
------------------------------------------------*/
extern void userDS12302_Init(void);


#endif
/**************************************************************************************************/

