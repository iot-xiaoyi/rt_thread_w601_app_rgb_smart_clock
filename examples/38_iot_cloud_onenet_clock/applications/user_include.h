#ifndef __USER_INCLUDE__H_
#define __USER_INCLUDE__H_

#include <stdlib.h>
#include <rtthread.h>

void send_mq_msg(char *topic_name, uint8_t * data, int len);

int parse_cmd(char *topic_name, uint8_t *data, int len);

#endif

