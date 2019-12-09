/*
 * File      : httpclient.c
 *
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author      Notes
 * 2018-07-20     flybreak     first version
 * 2018-09-05     flybreak     Upgrade API to webclient latest version
 */
#include <webclient.h>  /* 使用 HTTP 协议与服务器通信需要包含此头文件 */
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include <netdb.h>
#include <cJSON.h>
#include <finsh.h>
#include "user_include.h"
#include <time.h>
#include <stdint.h>
#include "ntp.h"

#define GET_HEADER_BUFSZ        1024        //头部大小
#define GET_RESP_BUFSZ          2048        //响应缓冲区大小
#define GET_URL_LEN_MAX         256         //网址最大长度
#define GET_URI                 "http://v.juhe.cn/weather/index?cityname=%E5%8C%97%E4%BA%AC&dtype=json&format=1&key=7022b77eecfce961ebf58268a656412e" //获取天气的 API

#define POST_DATA_WEATHER    "{\"weather\":\"%s\"}"


// http://v.juhe.cn/weather/index?cityname=%E5%8C%97%E4%BA%AC&dtype=json&format=1&key=7022b77eecfce961ebf58268a656412e
// {"resultcode":"200","reason":"successed!","result":{"sk":{"temp":"-4","wind_direction":"南风","wind_strength":"1级","humidity":"90%","time":"01:34"},"today":{"temperature":"-4℃~9℃","weather":"雾转晴","weather_id":{"fa":"18","fb":"00"},"wind":"西北风4-5级","week":"星期二","city":"北京","date_y":"2019年12月10日","dressing_index":"较冷","dressing_advice":"建议着厚外套加毛衣等服装。年老体弱者宜着大衣、呢外套加羊毛衫。","uv_index":"中等","comfort_index":"","wash_index":"较不宜","travel_index":"较不宜","exercise_index":"较不宜","drying_index":""},"future":{"day_20191210":{"temperature":"-4℃~9℃","weather":"雾转晴","weather_id":{"fa":"18","fb":"00"},"wind":"西北风4-5级","week":"星期二","date":"20191210"},"day_20191211":{"temperature":"-6℃~4℃","weather":"晴","weather_id":{"fa":"00","fb":"00"},"wind":"西北风3-5级","week":"星期三","date":"20191211"},"day_20191212":{"temperature":"-5℃~5℃","weather":"晴","weather_id":{"fa":"00","fb":"00"},"wind":"西南风微风","week":"星期四","date":"20191212"},"day_20191213":{"temperature":"-3℃~9℃","weather":"晴","weather_id":{"fa":"00","fb":"00"},"wind":"北风3-5级","week":"星期五","date":"20191213"},"day_20191214":{"temperature":"-5℃~6℃","weather":"晴转多云","weather_id":{"fa":"00","fb":"01"},"wind":"南风微风","week":"星期六","date":"20191214"},"day_20191215":{"temperature":"-5℃~5℃","weather":"晴","weather_id":{"fa":"00","fb":"00"},"wind":"西南风微风","week":"星期日","date":"20191215"},"day_20191216":{"temperature":"-3℃~9℃","weather":"晴","weather_id":{"fa":"00","fb":"00"},"wind":"北风3-5级","week":"星期一","date":"20191216"}}},"error_code":0}

int user_weather_data_report(char *data)
{
    char send_data[256] = { 0x00 };

    rt_sprintf(send_data, POST_DATA_WEATHER, data);
    rt_kprintf("send_data is %s\r\n", send_data);
    send_mq_msg("$dp", send_data, strlen(send_data));
}


/* 天气数据解析 */
void weather_data_parse(rt_uint8_t *data)
{
    cJSON *root = RT_NULL, *object = RT_NULL, *item = RT_NULL, *pvalue = RT_NULL;
    root = cJSON_Parse((const char *)data);
    if (!root)
    {
        rt_kprintf("No memory for cJSON root!\n");
        return;
    }
    rt_kprintf("%s", data);
    object = cJSON_GetObjectItem(root, "result");
    item = cJSON_GetObjectItem(object, "today");
    pvalue =cJSON_GetObjectItem(item, "weather");
    rt_kprintf("\r\nweather:%s\r\n ", pvalue->valuestring);
    user_weather_data_report(pvalue->valuestring);

    pvalue =cJSON_GetObjectItem(item, "temperature");
    rt_kprintf("\r\temperature:%s ", pvalue->valuestring);
    if (root != RT_NULL)
        cJSON_Delete(root);
}

void weather(void)
{
    rt_uint8_t *buffer = RT_NULL;
    int resp_status;
    struct webclient_session *session = RT_NULL;
    char *weather_url = RT_NULL;
    int content_length = -1, bytes_read = 0;
    int content_pos = 0;
    /* 为 weather_url 分配空间 */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* 拼接 GET 网址 */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, "%s", GET_URI);
    /* 创建会话并且设置响应的大小 */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }
    /* 发送 GET 请求使用默认的头部 */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }
    /* 分配用于存放接收数据的缓冲 */
    buffer = rt_calloc(1, GET_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("No memory for data receive buffer!\n");
        goto __exit;
    }
    content_length = webclient_content_length_get(session);
    if (content_length < 0)
    {
        /* 返回的数据是分块传输的. */
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
        }while (1);
    }
    else
    {
        do
        {
            bytes_read = webclient_read(session, buffer,
                                        content_length - content_pos > GET_RESP_BUFSZ ?
                                        GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
            content_pos += bytes_read;
        }while (content_pos < content_length);
    }
    /* 天气数据解析 */
    weather_data_parse(buffer);
__exit:
    /* 释放网址空间 */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* 关闭会话 */
    if (session != RT_NULL)
        webclient_close(session);
    /* 释放缓冲区空间 */
    if (buffer != RT_NULL)
        rt_free(buffer);
}

// MSH_CMD_EXPORT(weather, Get weather by webclient);


void weather_task_thread(void* arg)
{
    int a =0;
	uint8_t *p = &a;
	USER_TIME_S current_time = { 0x00 };

    // user_dev_time_flash_read();
    while (4 != user_get_connect_status()->connect_status)
    {
        rt_thread_delay(1);
    }
    
    rt_thread_delay(3);
    rt_kprintf("################################### start to get weather");
    weather();

	while ( 1 )
	{
		user_get_time(&current_time);
        if ((0 == current_time.hour)  && (0 == current_time.minute) && (0 == current_time.second) )
        {
            rt_kprintf("########################## sync weather and ntp time #####################\r\n");
            weather();
            user_sntp_time_synced();
        }

        rt_thread_mdelay(1000);
	}
}

void weather_task_start(void)
{
    rt_thread_t tid;

    // /* create the ambient light data upload thread */
    // tid = rt_thread_create("local_weather",
    //                        weather_task_thread,
    //                        RT_NULL,
    //                        10*1024,
    //                        RT_THREAD_PRIORITY_MAX / 3 - 1,
    //                        5);
    // if (tid)
    // {
    //     rt_thread_startup(tid);
    // }
    weather_task_thread(0);
}

