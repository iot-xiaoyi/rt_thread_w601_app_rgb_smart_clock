#include "user_include.h"
#include "cJSON.h"
#include "string.h"
#include "stdio.h"

#define POST_DATA_TIMER   "{\"temp\":25,\"temp_mode\":1,\"humi\":34,\"time_mode\":1,\"time_show_mode\":1,\"voice\":1,\"timer_set\":[{\"id\":1,\"enable\":1,\"time\":\"07:10\",\"repeat\":1,\"week\":\"1 2 3 4 5\"}]}"


int parse_cmd(char *topic_name, uint8_t *data, int len)
{
    char res_buf[512] = { 0 };
	int value = 0;
    cJSON *json;

    data[len] = 0x00;
    rt_kprintf("recv data is %s, len is:%d\r\n", data, len);
    
    json = cJSON_Parse(data);

    cJSON *pName = cJSON_GetObjectItem(json, "name");
    cJSON *pValue = cJSON_GetObjectItem(json, "value");

    rt_kprintf("name is:%s, value is:%d\r\n", pName->valuestring, pValue->valueint);

    if (0 == rt_strncmp("timer_del", pName->valuestring, strlen(pName->valuestring)))
    {
        rt_kprintf("delete timer, index is %d\r\n", pValue->valueint);
        send_mq_msg(topic_name, POST_DATA_TIMER, strlen(POST_DATA_TIMER));

    }else if(0 == rt_strncmp("timer_set", pName->valuestring, strlen(pName->valuestring)))
    {
        rt_kprintf("modify/add timer\r\n");
    }


    cJSON_Delete(json);

	// send_mq_msg(topic_name, res_buf, strlen(res_buf));

    return 0;
}