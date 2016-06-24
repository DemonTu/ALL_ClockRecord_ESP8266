#ifndef __USERRFIDAPI_H
	#define __USERRFIDAPI_H

#define RFID_LED1_IO_MUX     PERIPHS_IO_MUX_MTDO_U
#define RFID_LED1_IO_NUM     15
#define RFID_LED1_IO_FUNC    FUNC_GPIO15


typedef struct
{
	uint16_t startFlag;
	TIME_STR startTime;
	TIME_STR endTime;
	uint16_t cntTimes;
	uint16_t endFlag;
}PARASAVE_STR;	// 总共16Byte

typedef struct
{
	uint32_t saveFlag;	 
	uint32_t currentAddr;
	uint8_t  deviceID[32];
}SYSTEMPARA_STR;	// 总共16Byte

extern SYSTEMPARA_STR sysPara;
void userFlashPro_Init(void);

void sysTemParaSave(void);


#endif	
