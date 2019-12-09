#include <stdio.h>

char *data1 = "Mon Dec  9 17:16:24 2019";
char *data = "Mon-Dec-9-17:16:24-2019";

int main11(void)
{
    char month[4] = { 0x00 };
    char week[4] = { 0x00 };
    char day[4] = {0x00 };
    char hour[4] = { 0x00 };
    char minute[4] = {0x00 };
    char second[4] = {0x00 };
    int year[4] = {0x00 };

    sscanf(data1, "%[^ ] %[^ ] %[^ ] %[^:]:%[^:]:%[^ ] %s", week, month, day, hour, minute, second, year);
    printf("get local current time: %s-%s-%s  %s  %s:%s:%s\r\n", year, month, day, week, hour, minute, second );

}