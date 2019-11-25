/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-01     ZYLX         first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <stdlib.h>
#include "wm_regs.h"
#include <string.h>
#include "bsp.h"

#define DBG_TAG "pm25"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define ADC_DEV_NAME        "adc"  /* ADC 设备名称 */
#define ADC_DEV_CHANNEL     6       /* ADC 通道 */
#define REFER_VOLTAGE       330         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 10)   /* 转换位数为10位 */

#define PM25_LED_POWER_PIN  7

static rt_adc_device_t adc_dev = RT_NULL;            /* ADC 设备句柄 */  

void pm25_led_init(void)
{
    rt_pin_mode(PM25_LED_POWER_PIN, PIN_MODE_OUTPUT);
}

void pm25_led_power(rt_bool_t enable)
{
    if (RT_TRUE == enable)
    {
        rt_pin_write(PM25_LED_POWER_PIN, PIN_LOW);
    }else
    {
        rt_pin_write(PM25_LED_POWER_PIN, PIN_HIGH);
    }
    
}

int pm25_init(void)
{
    pm25_led_init();

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }

    /* 使能设备 */
    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);

    return 0;
}

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

float pm25_read(void)
{
    rt_uint32_t value, vol;
    int delayTime=280;
    int delayTime2=40;

    pm25_led_power(RT_TRUE);
    rt_hw_us_delay(delayTime);
    /* 读取采样值 */
    value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
    rt_hw_us_delay(delayTime2);
    pm25_led_power(RT_FALSE);
    rt_hw_us_delay(9680);

    rt_kprintf("the value is :%d \n", value);

    /* 转换为对应电压值 */
    vol = value * REFER_VOLTAGE / CONVERT_BITS;
    rt_kprintf("the voltage is :%d.%02d \n", vol / 100, vol % 100);

    int val = ((value/1024)-0.0356)*120000*0.035;
    rt_kprintf("the result is :%d \n", val);

    return (float)vol;
}
