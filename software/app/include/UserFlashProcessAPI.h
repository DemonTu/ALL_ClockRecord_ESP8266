#ifndef __USERRFIDAPI_H
	#define __USERRFIDAPI_H

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

void userParaSave(PARASAVE_STR *para);
uint8_t userParaRead(PARASAVE_STR *para, uint32_t recordCnt);
void userTempParaSave(PARASAVE_STR *para);
uint32_t UserGetAllRecordNum(void);


#endif	
