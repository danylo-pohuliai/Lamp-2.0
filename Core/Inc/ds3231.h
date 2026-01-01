#ifndef DS3231_H
#define DS3231_H

#include "stm32f1xx_hal.h"

#define DS3231_ADDR (0x68 << 1)

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} RTC_Time_t;

uint8_t DS3231_Init(I2C_HandleTypeDef *hi2c);
void DS3231_GetTime(RTC_Time_t *time);
void DS3231_SetTime(RTC_Time_t *time);

#endif
