/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-01     brown         first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#include <onenet.h>
#include <string.h>
#include <easyflash.h>
#include <fal.h>
#include "wifi_config.h"
#include "onenet.h"
#include "user_include.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED_PIN PIN_LED_R
#define NUMBER_OF_UPLOADS 100

static rt_mq_t mq_send = NULL;
typedef struct _mq_send_msg_t{
    rt_uint8_t *data_ptr;    
    rt_uint32_t data_size;   
	char topic_name[64];
}mq_send_msg_t;


static void onenet_cmd_rsp_cb(char *topic_name, uint8_t *recv_data, size_t recv_size);

int main(void)
{
    /* 配置 wifi 工作模式 */
    rt_wlan_set_mode(RT_WLAN_DEVICE_STA_NAME, RT_WLAN_STATION);

    /* register the wlan ready callback function */
    rt_wlan_register_event_handler(RT_WLAN_EVT_READY, (void ( *)(int , struct rt_wlan_buff *, void *))onenet_mqtt_init, RT_NULL);
	
    /* 初始化 wlan 自动连接 */
    wlan_autoconnect_init();
    /* 使能 wlan 自动连接 */
    rt_wlan_config_autoreconnect(RT_TRUE);
	
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED_PIN, PIN_LOW);

    fal_init();
    easyflash_init();

	onenet_network_config();

    timer_task_start();

    return 0;
}

#define POST_DATA    "{\"power\":\"10\",\"color\":\"%d\"}"
#define POST_DATA1    "{\"power\":\"10\",\"color\":\"1\"}"
#define POST_DATA2    "{\"power\":\"10\",\"color\":\"2\"}"
#define POST_DATA3    "{\"power\":\"10\",\"color\":\"3\"}"
#define POST_DATA4    "{\"temp\":25,\"temp_mode\":1,\"humi\":34,\"time_mode\":1,\"time_show_mode\":1,\"voice\":1,\"timer_set\":[{\"id\":0,\"enable\":1,\"time\":\"09:10\",\"repeat\":0,\"week\":\"1 2 3\"},{\"id\":1,\"enable\":1,\"time\":\"07:10\",\"repeat\":1,\"week\":\"1 2 3 4 5\"}]}"

static void onenet_upload_entry(void *parameter)
{
	mq_send_msg_t send_msg = { 0x00 };
	// led_blue_on();
	send_mq_msg("$dp", (uint8_t *)POST_DATA4, rt_strlen((char *)POST_DATA4));
	
	while (1)
	{
		rt_memset(&send_msg, 0x00, sizeof(send_msg));
		if (RT_EOK == rt_mq_recv(mq_send, &send_msg, sizeof(send_msg), RT_WAITING_FOREVER))
		{
			if (onenet_mqtt_upload_data(send_msg.topic_name, (const char *)send_msg.data_ptr))
			{
				rt_kprintf("upload has an error, stop uploading\n");
				break;
			}
			else
			{
				if (NULL==rt_strstr(send_msg.topic_name, "$dp")) 
				{
					send_mq_msg("$dp", send_msg.data_ptr, send_msg.data_size);
				}
				rt_kprintf("buffer : %s\ntopic_name is:%s", send_msg.data_ptr, send_msg.topic_name);
				rt_free(send_msg.data_ptr);
			}
		}
		rt_thread_mdelay(50);
	}
	exit:
	rt_kprintf("upload thread exit!!!");
}

void onenet_upload_cycle(void)
{
    rt_thread_t tid;

    /* set the command response call back function */
    onenet_set_cmd_rsp_cb(onenet_cmd_rsp_cb);
	/* init send queue */
	mq_send = rt_mq_create("mq_send", sizeof(mq_send_msg_t), 10, RT_IPC_FLAG_FIFO);

    /* create the ambient light data upload thread */
    tid = rt_thread_create("onenet_send",
                           onenet_upload_entry,
                           RT_NULL,
                           2 * 1024,
                           RT_THREAD_PRIORITY_MAX / 3 - 1,
                           5);
    if (tid)
    {
        rt_thread_startup(tid);
    }
}

rt_err_t onenet_port_save_device_info(char *dev_id, char *api_key)
{
    EfErrCode err = EF_NO_ERR;

    /* save device id */
    err = ef_set_and_save_env("dev_id", dev_id);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save device info(dev_id : %s) failed!\n", dev_id);
        return -RT_ERROR;
    }

    /* save device api_key */
    err = ef_set_and_save_env("api_key", api_key);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save device info(api_key : %s) failed!\n", api_key);
        return -RT_ERROR;
    }

    /* save already_register environment variable */
    err = ef_set_and_save_env("already_register", "1");
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save already_register failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t onenet_port_get_register_info(char *dev_name, char *auth_info)
{
    rt_uint32_t mac_addr[2] = {0};
    EfErrCode err = EF_NO_ERR;

    /* get mac addr */
    if (rt_wlan_get_mac((rt_uint8_t *)mac_addr) != RT_EOK)
    {
        rt_kprintf("get mac addr err!! exit\n");
        return -RT_ERROR;
    }

    /* set device name and auth_info */
    rt_snprintf(dev_name, ONENET_INFO_AUTH_LEN, "%d%d", mac_addr[0], mac_addr[1]);
    rt_snprintf(auth_info, ONENET_INFO_AUTH_LEN, "%d%d", mac_addr[0], mac_addr[1]);

    /* save device auth_info */
    err = ef_set_and_save_env("auth_info", auth_info);
    if (err != EF_NO_ERR)
    {
        rt_kprintf("save auth_info failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t onenet_port_get_device_info(char *dev_id, char *api_key, char *auth_info)
{
    char *info = RT_NULL;

    /* get device id */
    info = ef_get_env("dev_id");
    if (info == RT_NULL)
    {
        rt_kprintf("read dev_id failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(dev_id, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    /* get device api_key */
    info = ef_get_env("api_key");
    if (info == RT_NULL)
    {
        rt_kprintf("read api_key failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(api_key, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    /* get device auth_info */
    info = ef_get_env("auth_info");
    if (info == RT_NULL)
    {
        rt_kprintf("read auth_info failed!\n");
        return -RT_ERROR;
    }
    else
    {
        rt_snprintf(auth_info, ONENET_INFO_AUTH_LEN, "%s", info);
    }

    return RT_EOK;
}

rt_bool_t onenet_port_is_registed(void)
{
    char *already_register = RT_NULL;

    /* check the device has been registered or not */
    already_register = ef_get_env("already_register");
    if (already_register == RT_NULL)
    {
        return RT_FALSE;
    }

    return already_register[0] == '1' ? RT_TRUE : RT_FALSE;
}

void send_mq_msg(char *topic_name, uint8_t * data, int len)
{
	mq_send_msg_t msg_send = { 0x00 };
	
	/* user have to use rt_malloc malloc memory for response data */
    msg_send.data_ptr = (uint8_t *) rt_malloc(len + 1);
	rt_memset(msg_send.data_ptr, 0x00, len + 1);
	
	rt_memcpy(msg_send.data_ptr, data, len);
    rt_strncpy(msg_send.topic_name, topic_name, rt_strlen(topic_name));
    msg_send.data_size = len;
	
	rt_mq_send(mq_send,(void*)&msg_send, sizeof(mq_send_msg_t));
}

static void onenet_cmd_rsp_cb(char *topic_name, uint8_t *recv_data, size_t recv_size)
{
    parse_cmd(topic_name, recv_data, recv_size );
    rt_memset(recv_data, 0x00, recv_size);
}
