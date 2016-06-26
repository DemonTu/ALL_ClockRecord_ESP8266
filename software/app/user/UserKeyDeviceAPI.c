#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"

#include "driver/gpio16.h"

#include "user_interface.h"

LOCAL os_timer_t key_timer;

LOCAL void ICACHE_FLASH_ATTR
keyScan(uint8_t flag)
{
	static uint8_t keyValue = 0;
	keyValue <<= 1; 
	if (0 == gpio16_input_get())
	{
		keyValue |= 1;
	}
	switch(keyValue)
	{
		case 0x0f:
			os_printf("keydown\r\n");
			break;
		case 0xff:
			os_printf("keylong\r\n");
			break;
		case 0xf0:
			os_printf("keyup\r\n");
			break;
		default:
			break;
	}
}



void ICACHE_FLASH_ATTR
userKeyDevice_Init(void)
{
	gpio16_input_conf();
	
	/* °´¼üÉ¨Ãè */
	os_timer_disarm(&key_timer);
	os_timer_setfn(&key_timer, (os_timer_func_t *)keyScan, 1);
	os_timer_arm(&key_timer, 50, 1);
	os_printf("key init\r\n");
}
