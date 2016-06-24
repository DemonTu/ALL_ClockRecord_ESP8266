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
*	ǰ2048K-16K Ϊ������ 
*	����ȥ2048kΪ�û������� 
*   ���16kΪϵͳ������ 
*******************************************************************/ 
#define SYSPARA_START_ADDR		((4096-16-2048)*1024)
#define PARASVAE_START_ADDR		(SYSPARA_START_ADDR+16*1024)
#define PARASVAE_END_ADDR		(PARASVAE_START_ADDR+2048*1024)

typedef struct
{
	uint32_t currentSaveAddr;
	uint32_t getReadParaNum;
}ParaRecordInfo_Str;

ParaRecordInfo_Str RecordInfo; // ������¼�ľ�� 

/**********************************************************
* 
* ϵͳ������������
*
***********************************************************/
SYSTEMPARA_STR sysPara;

void ICACHE_FLASH_ATTR
sysTemParaSave(void)
{
	spi_flash_erase_sector(SYSPARA_START_ADDR/4096);
	spi_flash_write(SYSPARA_START_ADDR, (uint32_t *)&sysPara, sizeof(SYSTEMPARA_STR));	
}

LOCAL void ICACHE_FLASH_ATTR
sysTemParaInit()
{
	spi_flash_read(SYSPARA_START_ADDR, (uint32_t *)&sysPara, sizeof(SYSTEMPARA_STR));
	if (sysPara.saveFlag != 0xaabbccdd)
	{
		sysPara.saveFlag    = 0xaabbccdd;
		sysPara.currentAddr = PARASVAE_START_ADDR;
		spi_flash_erase_sector(SYSPARA_START_ADDR/4096);
		spi_flash_write(SYSPARA_START_ADDR, (uint32_t *)&sysPara, sizeof(SYSTEMPARA_STR));
	}
}
/***************** END sysTemPara ********************************/

/****************************************************************
*=================	�û�ʹ�ò����Ĳ�ѯ��������� ====================	 
*
*�ṩ���м�¼�ɲ�ѯ, ��Ҫ���뽫��ѯ�ļ�¼��Ŀ����ȡ���µļ�¼
*
*************************************************************/

void ICACHE_FLASH_ATTR
userParaSave(PARASAVE_STR *para)
{
	if ((sysPara.currentAddr%4096) == 0)
	{
		spi_flash_erase_sector(sysPara.currentAddr/4096);
	}
	spi_flash_write(sysPara.currentAddr, (uint32_t *)para, sizeof(PARASAVE_STR));

	sysPara.currentAddr += sizeof(PARASAVE_STR);
	sysTemParaSave();
}

void ICACHE_FLASH_ATTR
userParaRead(PARASAVE_STR *para, uint32_t recordCnt)
{	
	uint32_t readAddr;
	readAddr = sysPara.currentAddr-recordCnt*sizeof(PARASAVE_STR);
	if (readAddr < PARASVAE_START_ADDR)
	{
		return;
	}
	spi_flash_read(readAddr, (uint32_t *)para, sizeof(PARASAVE_STR));
}


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
/*��ʱɨ�� rfid*/
	os_timer_disarm(&flash_timer);
	os_timer_setfn(&flash_timer, (os_timer_func_t *)UserFlashProcess, 1);
	os_timer_arm(&flash_timer, 500, 1);
}
