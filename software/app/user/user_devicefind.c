/******************************************************************************
 * Copyright 2015-2016  CHRTQY system (TuQianYin)
 *
 * FileName: user_devicefind.c
 *
 * Description: Find your hardware's information while working any mode.
 *
 * Modification history:
 *     2016/6/12, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#include "espconn.h"
//#include "user_json.h"
#include "user_devicefind.h"

// user device
#include "UserDS1302DriverAPI.h"
#include "UserFlashProcessAPI.h"
#include "UserKeyDeviceAPI.h"
#include "userSensorDetection.h"

/*---------------------------------------------------------------------------*/
LOCAL struct espconn ptrespconn;

/*--------------------------------------------------------------------
*  crc(2byte) head(1byte)	cmd(1byte)	len(2byte)	data(len byte)
*  注: crc只包含data
*--------------------------------------------------------------------*/
typedef struct
{
	uint16_t crc;
	uint8_t  head;
	uint8_t  cmd;
	uint16_t len;
//    uint8_t *dat; 	
}DataStr;

#define FIND_DEVICE			0x80
#define SET_DEVICE_ID		0x81
#define SYNC_SYSTEM_TIME	0x82
#define READ_DEVICE_PARA	0x83

#define ACK_OK				0x10
#define ACK_ERROR			0x11
/******************************************************************************
 * FunctionName : udpDataPacket
 * Description  : Packeting udp data to send
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
udpDataPacket(uint8_t *sendBuf, uint16_t len, uint8_t cmd)
{
	remot_info *premot = NULL;
	uint16_t length=0;
	uint8_t sendDeviceBuffer[60];
	uint16_t crcTemp;
	/* crc */
	
	sendDeviceBuffer[2] = 0xAA;
	sendDeviceBuffer[3] = cmd;
	//sendDeviceBuffer[4] = len;
	U16_To_BigEndingBuf(&sendDeviceBuffer[4], len);
	length += sizeof(DataStr);
	if (len)
	{
		os_memcpy(&sendDeviceBuffer[length], sendBuf, len);
		length += len;
	}
	crcTemp = Crc16_1021_Sum(&sendDeviceBuffer[2], (length-2));
	U16_To_BigEndingBuf(sendDeviceBuffer, crcTemp);
	
	if (espconn_get_connection_info(&ptrespconn, &premot, 0) != ESPCONN_OK)
	{
		return;
	}
	os_memcpy(ptrespconn.proto.udp->remote_ip, premot->remote_ip, 4);
	ptrespconn.proto.udp->remote_port = premot->remote_port;
	espconn_sent(&ptrespconn, sendDeviceBuffer, length);
}

/******************************************************************************
 * FunctionName : user_devicefind_recv
 * Description  : Processing the received data from the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_devicefind_recv(void *arg, char *pusrdata, unsigned short length)
{
    char DeviceBuffer[60] = {0};
	unsigned short datLen=0;
    char hwaddr[6];
    struct ip_info ipconfig;
	DataStr datAnalyze;

	uint16_t crcTemp;
	char sendFlag=0;

    if (wifi_get_opmode() != STATION_MODE) {
        wifi_get_ip_info(SOFTAP_IF, &ipconfig);
        wifi_get_macaddr(SOFTAP_IF, hwaddr);

        if (!ip_addr_netcmp((struct ip_addr *)ptrespconn.proto.udp->remote_ip, &ipconfig.ip, &ipconfig.netmask)) {
            wifi_get_ip_info(STATION_IF, &ipconfig);
            wifi_get_macaddr(STATION_IF, hwaddr);
        }
    } else {
        wifi_get_ip_info(STATION_IF, &ipconfig);
        wifi_get_macaddr(STATION_IF, hwaddr);
    }

    if (pusrdata == NULL) {
        return;
    }
	else if (length < sizeof(DataStr))
	{
		udpDataPacket(NULL, 0, ACK_ERROR);
		return;
	}
	datAnalyze.crc  = BigEndingBuf_To_U16(pusrdata);
	crcTemp = Crc16_1021_Sum((pusrdata+2), length-2);
	if (datAnalyze.crc != crcTemp)
	{
		udpDataPacket(NULL, 0, ACK_ERROR);
		os_printf("crc=0x%x", crcTemp);
		return;
	}
	datAnalyze.head = pusrdata[2];
	datAnalyze.cmd  = pusrdata[3];
	datAnalyze.len  = BigEndingBuf_To_U16(pusrdata+4);
	switch(datAnalyze.cmd)
	{
		case FIND_DEVICE:
			if (0 == datAnalyze.len)
			{
				/* 初始通信用 */
				os_memcpy(DeviceBuffer, sysPara.deviceID, sizeof(sysPara.deviceID));		
				os_memcpy(&DeviceBuffer[sizeof(sysPara.deviceID)], hwaddr, 6);		
				os_memcpy(&DeviceBuffer[sizeof(sysPara.deviceID)+6], &ipconfig.ip, 4);		
				datLen = sizeof(sysPara.deviceID)+10;
				sendFlag = 1;
			}
			else if ((datAnalyze.len == sizeof(sysPara.deviceID)) &&
				(0==os_memcmp(sysPara.deviceID, &pusrdata[6], datAnalyze.len)))
			{
				/* 应答 */
				os_memcpy(&DeviceBuffer[0], hwaddr, 6);		
				os_memcpy(&DeviceBuffer[6], &ipconfig.ip, 4);	
				datLen = 10;
				sendFlag = 1;
			}
			break;
		case SET_DEVICE_ID:
			os_memcpy(sysPara.deviceID, &pusrdata[sizeof(datAnalyze)], datAnalyze.len);
			sysTemParaSave();
			datLen = 0;
			sendFlag = 1;
			break;
		case SYNC_SYSTEM_TIME:
			{
				TIME_STR timeTemp;
				
				timeTemp.year   = pusrdata[sizeof(DataStr)];
				timeTemp.month  = pusrdata[sizeof(DataStr)+1];
				timeTemp.data   = pusrdata[sizeof(DataStr)+2];
				timeTemp.hour   = pusrdata[sizeof(DataStr)+3];
				timeTemp.minute = pusrdata[sizeof(DataStr)+4];

				userDS1302WriteTime(&timeTemp);
				datLen = 0;
				sendFlag = 1;
			}
			break;
		case READ_DEVICE_PARA:
			{
				PARASAVE_STR paraTemp;
				uint32_t cnt;

				cnt = BigEndingBuf_To_U32(&pusrdata[6+datAnalyze.len-4]);
				if (0==os_memcmp(sysPara.deviceID, &pusrdata[6], datAnalyze.len-4) && cnt)
				{
					if (userParaRead(&paraTemp, cnt))
					{
						datLen = sizeof(PARASAVE_STR);
						U16_To_BigEndingBuf(DeviceBuffer, paraTemp.startFlag);
						os_memcpy(&DeviceBuffer[2], &paraTemp.startTime, sizeof(TIME_STR));
						os_memcpy(&DeviceBuffer[sizeof(TIME_STR)+2], &paraTemp.endTime, sizeof(TIME_STR));
						U16_To_BigEndingBuf(&DeviceBuffer[datLen-4], paraTemp.cntTimes);
						U16_To_BigEndingBuf(&DeviceBuffer[datLen-2], paraTemp.endFlag);
					}
					else
					{
						datLen = 0;
					}
					sendFlag = 1;
				}
				else
				{
					udpDataPacket(NULL, 0, ACK_ERROR);
					return;
				}
			}
			break;

		default:
			break;
	}
    if (sendFlag) 
    {
		udpDataPacket(DeviceBuffer, datLen, ACK_OK);
	}

}

/******************************************************************************
 * FunctionName : user_devicefind_init
 * Description  : the espconn struct parame init
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_devicefind_init(void)
{
    ptrespconn.type = ESPCONN_UDP;
    ptrespconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    ptrespconn.proto.udp->local_port = 1025;
    espconn_regist_recvcb(&ptrespconn, user_devicefind_recv);
    espconn_create(&ptrespconn);
}
