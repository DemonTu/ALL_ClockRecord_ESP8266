/******************************************************************************
 * Copyright 2016-2017 TUT Systems (TQY)
 *
 * FileName: UserRfidAPI.c
 *
 * Description: RFID demo's function realization
 *
 * Modification history:
 *     2016/1/25, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
//#include "mem.h"
#include "user_interface.h"
#include "GPIO.h"

#include "UserDS1302DriverAPI.h"
#include "UserFlashProcessAPI.h"


/******************************************************************* 
* 	flash size 4MByte 
*	前1024K-16K 为代码区 
*	接下去3072k为用户参数区 
*   最后16k为系统参数区 
*******************************************************************/ 
#define SENSORCNTTEMP_ADDR		((1024-16)*1024)
#define SYSPARA_START_ADDR		((4096-16-2048)*1024)
#define PARASVAE_START_ADDR		(SYSPARA_START_ADDR+16*1024)
#define PARASVAE_END_ADDR		(PARASVAE_START_ADDR+2048*1024)

typedef struct
{
	uint32_t currentSaveAddr;
	uint32_t getReadParaNum;
}ParaRecordInfo_Str;

ParaRecordInfo_Str RecordInfo; // 参数记录的句柄 

/**********************************************************
* 
* 系统参数操作函数
*
***********************************************************/
SYSTEMPARA_STR sysPara;

void ICACHE_FLASH_ATTR
sysTemParaSave(void)
{
	spi_flash_erase_sector(SYSPARA_START_ADDR/4096);
	spi_flash_write(SYSPARA_START_ADDR, (uint32 *)&sysPara, sizeof(SYSTEMPARA_STR));	
}

LOCAL void ICACHE_FLASH_ATTR
sysTemParaInit()
{
	uint32_t chipID = 0;
	uint8_t bufT[17]={0};
	PARASAVE_STR tempSaveCnt;

	chipID = system_get_chip_id();
	//os_printf("ID=0x%x", chipID);
	spi_flash_read(SYSPARA_START_ADDR, (uint32 *)&sysPara, sizeof(SYSTEMPARA_STR));
	if (sysPara.saveFlag != 0xaabbccee)
	{
		system_soft_wdt_stop();
		sysPara.saveFlag    = 0xaabbccee;
		sysPara.currentAddr = PARASVAE_START_ADDR;
		os_memcpy(sysPara.deviceID, (uint8_t *)&chipID, 4);
		
		spi_flash_erase_sector(SYSPARA_START_ADDR/4096);
		spi_flash_write(SYSPARA_START_ADDR, (uint32 *)&sysPara, sizeof(SYSTEMPARA_STR));
		system_soft_wdt_restart();
	}
// 读取断电记录	
	spi_flash_read(SENSORCNTTEMP_ADDR, (uint32 *)&tempSaveCnt, sizeof(PARASAVE_STR));
	if (tempSaveCnt.startFlag==0x5566 && tempSaveCnt.endFlag==0x7788)
	{
		userParaSave(&tempSaveCnt);
		spi_flash_erase_sector(SENSORCNTTEMP_ADDR/4096);
	}
	else if (tempSaveCnt.startFlag != 0xffff)
	{
		spi_flash_erase_sector(SENSORCNTTEMP_ADDR/4096);
	}
//===============test=======================
	//os_printf("addr=0x%x", sysPara.currentAddr);
//===========================================
}
/***************** END sysTemPara ********************************/

/****************************************************************
*=================	用户使用参数的查询、保存参数 ====================	 
*
*提供所有记录可查询, 需要传入将查询的记录数目，获取最新的记录
*
*******************************************************************/

void ICACHE_FLASH_ATTR
userParaSave(PARASAVE_STR *para)
{
	if ((sysPara.currentAddr%4096) == 0)
	{
		spi_flash_erase_sector(sysPara.currentAddr/4096);
	}
	spi_flash_write(sysPara.currentAddr, (uint32 *)para, sizeof(PARASAVE_STR));

	sysPara.currentAddr += sizeof(PARASAVE_STR);
	sysTemParaSave();
}

uint8_t ICACHE_FLASH_ATTR
userParaRead(PARASAVE_STR *para, uint32_t recordCnt)
{	
	uint32_t readAddr;
	if (sysPara.currentAddr < PARASVAE_START_ADDR+recordCnt*sizeof(PARASAVE_STR))
	{
		para = NULL;
		os_printf("NULL1\r\n");
		return 0;
	}
	else
	{
		readAddr = sysPara.currentAddr-recordCnt*sizeof(PARASAVE_STR);
		if (readAddr < PARASVAE_START_ADDR)
		{
			para = NULL;
			os_printf("NULL2\r\n");
			return 0;
		}
		spi_flash_read(readAddr, (uint32 *)para, sizeof(PARASAVE_STR));
	}
	return 1;
}

void ICACHE_FLASH_ATTR
userTempParaSave(PARASAVE_STR *para)
{
	spi_flash_write(SENSORCNTTEMP_ADDR, (uint32 *)para, sizeof(PARASAVE_STR));
}


uint32_t ICACHE_FLASH_ATTR
UserGetAllRecordNum(void)
{
	return ((sysPara.currentAddr-PARASVAE_START_ADDR)/sizeof(PARASAVE_STR) );
}
//===================================================
#define RFID_LED1_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define RFID_LED1_IO_NUM     15
#define RFID_LED1_IO_FUNC    FUNC_GPIO15

LOCAL os_timer_t flash_timer;

void ICACHE_FLASH_ATTR
UserFlashProcess(uint8_t flag)
{
	LOCAL uint8_t flashFlag =0;

	if (flashFlag){

		GPIO_OUTPUT_SET(GPIO_ID_PIN(RFID_LED1_IO_NUM),1);
		flashFlag = 0;
	}else{

		GPIO_OUTPUT_SET(GPIO_ID_PIN(RFID_LED1_IO_NUM),0);
		flashFlag = 1;
	}

}

/******************************************************************************
 * FunctionName : userFlashPro_Init
 * Description  : init RFID SPI GPIO
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
userFlashPro_Init(void)
{
	// GPIO init
	PIN_PULLUP_DIS(RFID_LED1_IO_MUX);
	PIN_FUNC_SELECT(RFID_LED1_IO_MUX, RFID_LED1_IO_FUNC);
	
	sysTemParaInit();
	//os_printf("test resd\r\n");
/*定时扫描 rfid*/
	os_timer_disarm(&flash_timer);
	os_timer_setfn(&flash_timer, (os_timer_func_t *)UserFlashProcess, 1);
	os_timer_arm(&flash_timer, 500, 1);
}

