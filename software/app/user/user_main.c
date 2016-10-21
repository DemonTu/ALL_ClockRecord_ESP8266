/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_devicefind.h"

#include "driver/uart.h"
// user device
#include "UserDS1302DriverAPI.h"
#include "UserFlashProcessAPI.h"
#include "UserKeyDeviceAPI.h"
#include "userSensorDetection.h"

#define DISCONNECT_TIME				15 // 30s 
void user_rf_pre_init(void)
{
}

void ICACHE_FLASH_ATTR
user_set_station_config(void)
{

	struct station_config stationConf;
	os_memset(&stationConf, 0, sizeof(stationConf));
	stationConf.bssid_set = 0; //need not check MAC address of AP
	os_memcpy(&stationConf.ssid, "FJXMYKD", 7);
	os_memcpy(&stationConf.password, "FJXMYKD456123", 13);

	wifi_station_set_config_current(&stationConf);
}

void wifi_handle_event_cb(System_Event_t *evt)
{
	//os_printf("event %x\n", evt->event);
	LOCAL uint8_t disconnectCnt = DISCONNECT_TIME;
	
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED:
			disconnectCnt = DISCONNECT_TIME;
			os_printf("connect to ssid %s, channel %d\n",
			evt->event_info.connected.ssid,
			evt->event_info.connected.channel);
			break;
		case EVENT_STAMODE_DISCONNECTED:
			os_printf("disconnect from ssid %s, reason %d %d\n", 
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason,
			disconnectCnt);
			if (disconnectCnt)
			{
				if (--disconnectCnt == 0)
				{
					os_printf("event %x\n", evt->event);
				//	wifi_station_set_reconnect_policy(FALSE); // 停止搜索，减少功耗
				}
			}
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			os_printf("mode: %d -> %d\n",
			evt->event_info.auth_change.old_mode,
			evt->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
			IP2STR(&evt->event_info.got_ip.ip),
			IP2STR(&evt->event_info.got_ip.mask),
			IP2STR(&evt->event_info.got_ip.gw));
			os_printf("\n");
			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			os_printf("station: " MACSTR "join, AID = %d\n",
				MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			os_printf("station: " MACSTR "leave, AID = %d\n",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			break;
		default:
			break;
	}
}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	
    os_printf("\r\nSDK version:%s\r\n", system_get_sdk_version());
	os_printf("system Freq=%dMhz\r\n", system_get_cpu_freq());
	//os_printf("heapsize=%d\r\n", system_get_free_heap_size());
// user device
	userFlashPro_Init();
	userDS12302_Init();
	userKeyDevice_Init();
	user_SensorDetection_Init();
	
//=======================================================
	/* 设置WiFi为STA模式，连接指定的AP */
	wifi_set_opmode(STATION_MODE);
	user_set_station_config();
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	
    os_printf("sta=%d\r\n", wifi_get_opmode());
//========================================================	
    /*Establish a udp socket to receive local device detect info.*/
    /*Listen to the port 1025, as well as udp broadcast.
    /*If receive a string of device_find_request, it rely its IP address and MAC.*/
    user_devicefind_init();
}

