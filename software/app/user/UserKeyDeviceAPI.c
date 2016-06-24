#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "driver/key.h"

#define USER_KEY_1_IO_MUX	  PERIPHS_IO_MUX_MTCK_U //   GPIO6不是通用IO口
#define USER_KEY_1_IO_NUM	  13
#define USER_KEY_1_IO_FUNC    FUNC_GPIO13

#define USER_KEY_2_IO_MUX	  PERIPHS_IO_MUX_GPIO2_U
#define USER_KEY_2_IO_NUM	  2
#define USER_KEY_2_IO_FUNC    FUNC_GPIO2

#define USER_KEY_3_IO_MUX	  PERIPHS_IO_MUX_GPIO0_U
#define USER_KEY_3_IO_NUM	  0
#define USER_KEY_3_IO_FUNC    FUNC_GPIO0

#define USER_KEY_5_IO_MUX	  PERIPHS_IO_MUX_U0RXD_U 
#define USER_KEY_5_IO_NUM	  3
#define USER_KEY_5_IO_FUNC    FUNC_GPIO3

#define USER_KEY_NUM		3  // GPIO16 保留
LOCAL struct single_key_param *single_key[USER_KEY_NUM];
LOCAL struct keys_param keys;
typedef struct 
{
	uint32 gpioName;
	uint32 gpioFunc;
	uint32 gpioId;
	key_function longPressFunc_cb;
	key_function shortPressFunc_cb;
}keyEventStr;

LOCAL void ICACHE_FLASH_ATTR
userKey1LongPress(void)
{

}
LOCAL void ICACHE_FLASH_ATTR
userKey1ShortPress(void)
{

}
LOCAL void ICACHE_FLASH_ATTR
userKey2LongPress(void)
{

}
LOCAL void ICACHE_FLASH_ATTR
userKey2ShortPress(void)
{

}LOCAL void ICACHE_FLASH_ATTR
userKey3LongPress(void)
{

}
LOCAL void ICACHE_FLASH_ATTR
userKey3ShortPress(void)
{

}LOCAL void ICACHE_FLASH_ATTR
userKey5LongPress(void)
{

}
LOCAL void ICACHE_FLASH_ATTR
userKey5ShortPress(void)
{

}

LOCAL keyEventStr keyIoInfo[USER_KEY_NUM] = {   
	{USER_KEY_1_IO_MUX,USER_KEY_1_IO_FUNC,USER_KEY_1_IO_NUM,userKey1LongPress, userKey1ShortPress},
//	{USER_KEY_2_IO_MUX,USER_KEY_2_IO_FUNC,USER_KEY_2_IO_NUM,userKey2LongPress, userKey2ShortPress},
	{USER_KEY_3_IO_MUX,USER_KEY_3_IO_FUNC,USER_KEY_3_IO_NUM,userKey3LongPress, userKey3ShortPress},
	{USER_KEY_5_IO_MUX,USER_KEY_5_IO_FUNC,USER_KEY_5_IO_NUM,userKey5LongPress, userKey5ShortPress},
};

void ICACHE_FLASH_ATTR
userKeyDevice_Init(void)
{
	uint8_t i;
	for(i=0; i<USER_KEY_NUM; i++){
		single_key[i] = key_init_single(keyIoInfo[i].gpioId, keyIoInfo[i].gpioName, keyIoInfo[i].gpioFunc,
                        	keyIoInfo[i].longPressFunc_cb, keyIoInfo[i].shortPressFunc_cb);
	}
    keys.key_num = USER_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);
	os_printf("key init\r\n");
}
