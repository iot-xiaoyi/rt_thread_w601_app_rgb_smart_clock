#ifndef __USER_INCLUDE__H_
#define __USER_INCLUDE__H_

#include <stdlib.h>
#include <rtthread.h>
#include "errno.h"
#include <stdint.h>

typedef struct _user_status_data_t{
    int connect_status; // 0:init, 1:连接路由器中，, 2:连接路由器成功， 3:连接路由器失败 4:连云成功
}user_status_data_t;

user_status_data_t *user_get_connect_status(void);

/********************** user_timer.c *************************/
#define LOCAL_TIME_MAX_NUM                     (2)
#define USER_TIME_NUM_TOKENS                   (30)

#define TIME_SYNC_PERIOD                        (60 * 60)


typedef struct _USER_TIMER_S{
    uint32_t    year;
    uint8_t     month;
    uint8_t     weekday;
    uint8_t     day;
    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
    uint32_t    timestamp;
}USER_TIME_S;

void user_sntp_time_synced( void );

void timer_task_start(void);


/******************** recv_cloud_data.c ***********************/
typedef struct _Time_UserTask_t{
    uint8_t id;
    uint8_t enable;
    uint8_t valid;
    uint8_t repeat;
    uint8_t weekday;
	uint8_t run_hour;
	uint8_t run_min;
    int run_day;
    int run_mon;
    int run_year;
    char cmd_buff[128];
}Time_UserTask_t;

extern Time_UserTask_t local_time_task[];

void send_mq_msg(char *topic_name, uint8_t * data, int len);

int parse_cmd(char *topic_name, uint8_t *data, int len);

int user_time_task_result_report(void);

int bsp_init(void);

void beep_open(void);

void beep_close(void);

#endif

