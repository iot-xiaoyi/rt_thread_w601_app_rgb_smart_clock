#include "user_include.h"
#include <time.h>
#include <stdint.h>
#include "ntp.h"

#define user_timer_log    rt_kprintf

int user_task_time_coming(char *cmd)
{
    user_timer_log("##################### Timer coming #####################\r\n");

    // user_timer_log("cmd is:%s", cmd);
    beep_open();
}

/* Callback function when MiCO UTC time in sync to NTP server */
void user_sntp_time_synced( void )
{
    time_t ntp_time = 0x00;
    ntp_time = ntp_sync_to_rtc(NULL);
}

int user_get_time( USER_TIME_S *current_time )
{
    int err = -1;
    static int log_num = 0;
    time_t now;
    char month[4] = { 0x00 };
    char week[4] = { 0x00 };
    char day[4] = {0x00 };
    char hour[4] = { 0x00 };
    char minute[4] = {0x00 };
    char second[4] = {0x00 };
    int year[4] = {0x00 };

    /* output current time */
    now = time(RT_NULL);
    // rt_kprintf("%s", ctime(&now));  //Mon Dec  9 13:14:55 2019
    sscanf(ctime(&now), "%[^ ] %[^ ] %[^ ] %[^:]:%[^:]:%[^ ] %s", week, month, day, hour, minute, second, year);
    // user_timer_log("get local current time:%s-%s-%s  %s %s:%s:%s\r\n", week, month, day, hour, minute, second, year);

    current_time->second = atoi(second);
    current_time->minute = atoi(minute);
    current_time->hour   = atoi(hour);
    if( current_time->hour>23 )
    {
        current_time->hour = current_time->hour%24;
    }
    current_time->day     = atoi(day);
    current_time->year    = atoi(year);
    //week
    if (NULL != rt_strstr(week, "Mon"))
    {
        current_time->weekday = 1;
    }else if (NULL != rt_strstr(week, "Tue"))
    {
        current_time->weekday = 2;
    }else if (NULL != rt_strstr(week, "Wed"))
    {
        current_time->weekday = 3;
    }else if (NULL != rt_strstr(week, "Thu"))
    {
        current_time->weekday = 4;
    }else if (NULL != rt_strstr(week, "Fri"))
    {
        current_time->weekday = 5; 
    }else if (NULL != rt_strstr(week, "Sat"))
    {
        current_time->weekday = 6;  
    }else if (NULL != rt_strstr(week, "Sun"))
    {
        current_time->weekday = 7; 
    }
    // month
    if (NULL != rt_strstr(month, "Jan"))
    {
        current_time->month = 1;
    }else if (NULL != rt_strstr(month, "Feb"))
    {
        current_time->month = 2;
    }else if (NULL != rt_strstr(month, "Mar"))
    {
        current_time->month = 3;
    }else if (NULL != rt_strstr(month, "Apr"))
    {
        current_time->month = 4;
    }else if (NULL != rt_strstr(month, "May"))
    {
        current_time->month = 5; 
    }else if (NULL != rt_strstr(month, "Jun"))
    {
        current_time->month = 6;  
    }else if (NULL != rt_strstr(month, "Jul"))
    {
        current_time->month = 7; 
    }else if (NULL != rt_strstr(month, "Aug"))
    {
        current_time->month = 8; 
    }else if (NULL != rt_strstr(month, "Sep"))
    {
        current_time->month = 9; 
    }else if (NULL != rt_strstr(month, "Oct"))
    {
        current_time->month = 10; 
    }else if (NULL != rt_strstr(month, "Nov"))
    {
        current_time->month = 11; 
    }else if (NULL != rt_strstr(month, "Dec"))
    {
        current_time->month = 12; 
    }
    if ( 0 == (log_num % 20) )
    {
        user_timer_log("current time: year:%d, month:%d, week:%d, day:%d, hour:%d, minute:%d, second:%d\r\n", \
                   current_time->year, current_time->month, current_time->weekday, current_time->day, current_time->hour, current_time->minute, current_time->second);
    }
    log_num++;

    return err;
}

//timer task run
void user_timer_task(USER_TIME_S *current_time, uint8_t i)
{
    if(1 == local_time_task[i].enable)
    {
        if(local_time_task[i].repeat == 0)  //only once
        { 
            if((local_time_task[i].run_year == current_time->year ) && \
                (local_time_task[i].run_mon == current_time->month ) && \
                (local_time_task[i].run_hour == current_time->hour ) && \
                (local_time_task[i].run_min == current_time->minute) && (current_time->second == 0))
            {
                local_time_task[i].enable = 0;
                //write flash
                //user_dev_time_flash_write(i, &local_time_task[i]);
                user_task_time_coming(local_time_task[i].cmd_buff);
                user_time_task_result_report();
            }
        }
        else
        {
            if(current_time->weekday == 1)
            {
                if((local_time_task[i].weekday & 0x01) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                        user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 2)
            {
                if((local_time_task[i].weekday & 0x02) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                        user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 3)
            {
                if((local_time_task[i].weekday & 0x04) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                    	user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 4)
            {
                if((local_time_task[i].weekday & 0x08) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                    	user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 5)
            {
                if((local_time_task[i].weekday & 0x10) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                    	user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 6)
            {
                if((local_time_task[i].weekday & 0x20) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                    	user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();
                    }
                }
            }
            else if(current_time->weekday == 7)
            {
                if((local_time_task[i].weekday & 0x40) > 0)
                {
                    if((local_time_task[i].run_hour == current_time->hour ) && (local_time_task[i].run_min == current_time->minute) && (current_time->second== 0))
                    {
                    	user_task_time_coming(local_time_task[i].cmd_buff);
                        user_time_task_result_report();

                    }
                }
            }
        }
    }
}

void timer_task_thread(void* arg)
{
	uint8_t i = 0;
	USER_TIME_S current_time = { 0x00 };

    rt_thread_delay(3);
    // user_dev_time_flash_read();
    user_sntp_time_synced();

	while ( 1 )
	{
		user_get_time(&current_time);
		for (i=0; i<LOCAL_TIME_MAX_NUM; i++)
		{
			user_timer_task(&current_time, i);
		}

        rt_thread_mdelay(500);
	}
}

void timer_task_start(void)
{
    rt_thread_t tid;

    /* create the ambient light data upload thread */
    tid = rt_thread_create("local_timer",
                           timer_task_thread,
                           RT_NULL,
                           2 * 1024,
                           RT_THREAD_PRIORITY_MAX / 3 - 1,
                           5);
    if (tid)
    {
        rt_thread_startup(tid);
    }
}