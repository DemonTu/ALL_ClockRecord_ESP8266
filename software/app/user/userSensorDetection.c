/******************************************************************************
 * Copyright 2016 DINDING (Demon)
 *
 * FileName: sensorDetection.c
 *
 * Description: read sensor count
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"

#include "userSensorDetection.h"
#include "UserDS1302DriverAPI.h"
#include "UserFlashProcessAPI.h"

//=======================================================
#if 1
#define SENSOR_IO_MUX     PERIPHS_IO_MUX_GPIO4_U
#define SENSOR_IO_NUM     4
#define SENSOR_IO_FUNC    FUNC_GPIO4
#else
#define SENSOR_IO_MUX     PERIPHS_IO_MUX_MTDI_U
#define SENSOR_IO_NUM     12
#define SENSOR_IO_FUNC    FUNC_GPIO12
#endif
LOCAL os_timer_t sensor_timer;
LOCAL os_timer_t read_sensor_timer;


LOCAL uint16_t sensorCnt;
LOCAL uint16_t readSensorTime;

LOCAL TIME_STR startTimes;
LOCAL TIME_STR endTimes;
#define READSENSOR_TIMES		60;	// 单位秒
/******************************************************************************
 * FunctionName : sensorParaRead_cb
 * Description  : 定时读取sensor参数
 * Parameters   : flag trashy
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
sensorParaRead_cb(uint8_t flag)
{
	PARASAVE_STR paraTemp;
		
    if (readSensorTime) 
	{
		if (--readSensorTime == 0)		
		{
			/* 结束计数的时间 */
			userDS1302ReadTime(&endTimes);
			
			/* 记录保存参数 */
			paraTemp.startFlag = 0x5566;
			paraTemp.startTime = startTimes;
			paraTemp.endTime   = endTimes;
			paraTemp.cntTimes  = sensorCnt;
			paraTemp.endFlag   = 0x7788;
			userParaSave(&paraTemp);
			sensorCnt = 0;
		}
    }

}

/******************************************************************************
 * FunctionName : sensor_20ms_cb
 * Description  : 20ms 延时去抖函数
 * Parameters   : flag trashy
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
sensor_20ms_cb(uint8_t flag)
{
    if (0 == GPIO_INPUT_GET(GPIO_ID_PIN(SENSOR_IO_NUM))) 
	{
		/* 计数 */
		if (0 == sensorCnt)
		{
			/* 开始计数的时间 */
			userDS1302ReadTime(&startTimes);
		}
        sensorCnt++;
		readSensorTime = READSENSOR_TIMES;	// 等待没有计数超时后，完成一次参数记录		
    }
	//os_printf("cnt=%d\r\n", sensorCnt);
	gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_IO_NUM), GPIO_PIN_INTR_NEGEDGE);
}

/******************************************************************************
 * FunctionName : sensorHandler
 * Description  : sensor interrupt handler
 * Parameters   : NULL
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
sensorHandler(void *para)
{
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

	//disable interrupt
	gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_IO_NUM), GPIO_PIN_INTR_DISABLE);

	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

     /* 延时20ms，去抖动 */
    os_timer_disarm(&sensor_timer);
    os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_20ms_cb, 1);
    os_timer_arm(&sensor_timer, 30, 0);
}

/******************************************************************************
 * FunctionName : user_SensorDetection_Init
 * Description  : sensor GPIO interrupt init
 * Parameters   : void
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_SensorDetection_Init(void)
{
    PIN_FUNC_SELECT(SENSOR_IO_MUX, SENSOR_IO_FUNC);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(SENSOR_IO_NUM));

	
	ETS_GPIO_INTR_ATTACH(sensorHandler, NULL);
	
	ETS_GPIO_INTR_DISABLE();

    //enable interrupt
    gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_IO_NUM), GPIO_PIN_INTR_NEGEDGE);

    ETS_GPIO_INTR_ENABLE();	

	sensorCnt = 0;
	readSensorTime = 0;
	os_printf("sensor init\r\n");

	os_timer_disarm(&read_sensor_timer);
	os_timer_setfn(&read_sensor_timer, (os_timer_func_t *)sensorParaRead_cb, 1);
	os_timer_arm(&read_sensor_timer, 1000, 1);
}

/******************************************************************************
 * FunctionName : userGetSensorCnt
 * Description  : get sensor count
 * Parameters   : void
 * Returns      : sensor count
*******************************************************************************/
uint16_t ICACHE_FLASH_ATTR
userGetSensorCnt(void)
{
	return sensorCnt;
}

/******************************************************************************
 * FunctionName : userClearSensorCnt
 * Description  : clear sensor count
 * Parameters   : void
 * Returns      : None
*******************************************************************************/
void ICACHE_FLASH_ATTR
userClearSensorCnt(void)
{ 
	sensorCnt = 0;
}

