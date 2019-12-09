/*
 * File      : onenet_mqtt.c
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-04-24     chenyong     first version
 */
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include <cJSON_util.h>

#include <paho_mqtt.h>

#include <onenet.h>
#include "wlan_mgnt.h"

#ifdef RT_USING_DFS
#include <dfs_posix.h>
#endif

int onenet_network_config(void)
{
	int result = 0;
//	struct rt_wlan_info info = { 0x00 };
//	
//	rt_err_t rt_wlan_start_ap_adv( &info, NULL);

	result = rt_wlan_connect("lxy2305", "123456789a");
    if (result != RT_EOK)
    {
        rt_kprintf("\r\nconnect ssid !\r\n");
        return result;
    }
}
