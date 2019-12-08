#include "user_include.h"
#include "cJSON.h"
#include "string.h"

int parse_cmd(char *topic_name, uint8_t *data, int len)
{
    char res_buf[128] = { 0 };
	int value = 0;
    cJSON *json;

    data[len] = 0x00;
    rt_kprintf("recv data is %s, len is:%d\r\n", data, len);
    
    json = cJSON_Parse(data);

    cJSON *pName = cJSON_GetObjectItem(json, "name");
    cJSON *pValue = cJSON_GetObjectItem(json, "value");

    rt_kprintf("name is:%s, value is:%d\r\n", pName->valuestring, pValue->valueint);

    cJSON_Delete(json);

	// send_mq_msg(topic_name, res_buf, strlen(res_buf));

    return 0;
}