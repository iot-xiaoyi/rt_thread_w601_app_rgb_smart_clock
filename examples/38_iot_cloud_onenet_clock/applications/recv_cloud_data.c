#include "user_include.h"
#include "cJSON.h"
#include "string.h"
#include "stdio.h"

#define POST_DATA_TIMER0   "{\"timer_set\":[{\"id\":0,\"enable\":%d,\"time\":\"%d:%d\",\"repeat\":%d,\"week\":\"%s\"}]}"  //\"week\":\"1 2 3 4 5\"
#define POST_DATA_TIMER1   "{\"timer_set\":[{\"id\":0,\"enable\":%d,\"time\":\"%d:%d\",\"repeat\":%d,\"week\":\"%s\"},{\"id\":1,\"enable\":%d,\"time\":\"%d:%d\",\"repeat\":%d,\"week\":\"%s\"}]}"  //\"week\":\"1 2 3 4 5\"

Time_UserTask_t local_time_task[LOCAL_TIME_MAX_NUM] = { 0x00 };

/**
 * @brief 返回数据到云端，便于小程序同步页面显示， 根据local_time_task[]中的内容上报
 * @param 数据下发的topic
 */ 
int user_time_task_result_res(char *topic_name)
{
    char send_data[256] = { 0x00 };
    char week[24] = { 0x00 };
    char week1[24] = { 0x00 };
    int i = 0;

    if ((0 ==local_time_task[0].valid) && (0 ==local_time_task[1].valid))
    {
        rt_strncpy(send_data, "{}", 2);
    }else if (1 ==local_time_task[1].valid)
    {
        int k = 0;
        for (i=0; i<8; i++)
        {
            if (0 != ((1<<i) & local_time_task[0].weekday) )
            {
                week[k++] = (i+1 + 0x30);
                week[k++] = ' ';
            }
        }
        k = 0;
        for (i=0; i<8; i++)
        {
            if (0 != ((1<<i) & local_time_task[1].weekday) )
            {
                week1[k++] = (i+1 + 0x30);
                week1[k++] = ' ';
            }
        }
        rt_sprintf(send_data, POST_DATA_TIMER1, local_time_task[0].enable, local_time_task[0].run_hour, local_time_task[0].run_min, \
            local_time_task[0].repeat, week, local_time_task[1].enable, local_time_task[1].run_hour, local_time_task[1].run_min,\
            local_time_task[1].repeat, week1);
    }else if (1 ==local_time_task[0].valid)
    {
        int k = 0;
        for (i=0; i<8; i++)
        {
            if (0 != ((1<<i) & local_time_task[0].weekday) )
            {
                week[k++] = (i+1 + 0x30);
                week[k++] = ' ';
            }
        }
        rt_kprintf("weekday is %x,  week is:%s\r\n", local_time_task[0].weekday, week);
        rt_sprintf(send_data, POST_DATA_TIMER0, local_time_task[0].enable, local_time_task[0].run_hour, local_time_task[0].run_min, \
           local_time_task[0].repeat, week);
    }
    
    send_mq_msg(topic_name, send_data, strlen(send_data));
}

int user_time_task_result_report(void)
{
    user_time_task_result_res("$dp");
}


int parse_cmd(char *topic_name, uint8_t *data, int len)
{
    char res_buf[512] = { 0 };
	int value = 0;
    int id = 0, i = 0, weekday = 0;
    cJSON *json;

    data[len] = 0x00;
    rt_kprintf("recv data is %s, len is:%d\r\n", data, len);
    
    json = cJSON_Parse(data);

    cJSON *pName = cJSON_GetObjectItem(json, "name");
    cJSON *pValue = cJSON_GetObjectItem(json, "value");

    rt_kprintf("name is:%s\r\n", pName->valuestring);

    if (0 == rt_strncmp("timer_del", pName->valuestring, strlen(pName->valuestring)))
    {
        rt_kprintf("delete timer, index is %d\r\n", pValue->valueint);
        /*一共两组， 删除任意一组，另一组id都为0，便于小程序增删操作
         *step1    清空收到的当前组数据
         *step2    判断收到的id是不是为1，如果是说明另一组id为0，满足要求；  如果不是1，需要将id为1的另一组复制到id为0(如果存在)
         *step3    根据最新的数组内容组包上传，如果无有效数据上传{}即可。
         */
        //step1
        id = pValue->valueint;
        local_time_task[id].valid = 0;
        //step2
        if (1 == id)
        {
            rt_kprintf("delete timer, index is %d\r\n", id);
            local_time_task[1].valid = 0;
        }else if (0 ==id)
        {
            if (1 == local_time_task[1].valid) //存在
            {
                rt_memcpy(&local_time_task[0], &local_time_task[1], sizeof(Time_UserTask_t));
            }
        }
        user_time_task_result_res(topic_name);

    }else if(0 == rt_strncmp("timer_set", pName->valuestring, strlen(pName->valuestring)))
    {
        rt_kprintf("modify/add timer\r\n");
        // {"name":"timer_set","value":{"id":575368182,"enable":1,"time":"12:15","repeat":1,"week":"1 3 4 6 "}}, len is:100
        cJSON *pData = cJSON_GetObjectItem(pValue, "id");
        id = pData->valueint;
        pData = cJSON_GetObjectItem(pValue, "enable");
        local_time_task[id].enable = pData->valueint;
        pData = cJSON_GetObjectItem(pValue, "time");
        sscanf(pData->valuestring, "%d:%d", &local_time_task[id].run_hour, &local_time_task[id].run_min);
        pData = cJSON_GetObjectItem(pValue, "repeat");
        rt_kprintf("repeat is:%d\r\n", pData->valueint);
        local_time_task[id].repeat = pData->valueint;
        local_time_task[id].valid = 1;
        pData = cJSON_GetObjectItem(pValue, "week");
        rt_kprintf("week is:%s\r\n", pData->valuestring);
        local_time_task[id].weekday = 0;
        for (i = 0; i < strlen(pData->valuestring); i++)
        {
            if (' ' != pData->valuestring[i])
            {
                weekday = pData->valuestring[i] - 0x30;
                rt_kprintf("get weekday, compose data: %d\r\n", weekday);
                local_time_task[id].weekday |= (1 << (weekday-1));
            }
        }

        user_time_task_result_res(topic_name);
    }

    cJSON_Delete(json);

	// send_mq_msg(topic_name, res_buf, strlen(res_buf));

    return 0;
}