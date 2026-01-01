#include "ds3231.h"

static I2C_HandleTypeDef *_ds3231_ui2c;

static uint8_t DecToBcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

static uint8_t BcdToDec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

static uint8_t CalculateDayOfWeek(uint8_t y, uint8_t m, uint8_t d) {
    uint16_t full_year = 2000 + y;
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    if (m < 3) {
        full_year -= 1;
    }

    int result = (full_year + full_year/4 - full_year/100 + full_year/400 + t[m-1] + d) % 7;

    if (result == 0) return 7;
    return result;
}

uint8_t DS3231_Init(I2C_HandleTypeDef *hi2c) {
    _ds3231_ui2c = hi2c;
    if (HAL_I2C_IsDeviceReady(_ds3231_ui2c, DS3231_ADDR, 5, 1000) != HAL_OK) {
        return 0;
    }
    return 1;
}

void DS3231_SetTime(RTC_Time_t *time) {
    uint8_t buf[7];

    if (time->day > 0 && time->month > 0) {
        time->day_of_week = CalculateDayOfWeek(time->year, time->month, time->day);
    }

    buf[0] = DecToBcd(time->seconds);
    buf[1] = DecToBcd(time->minutes);
    buf[2] = DecToBcd(time->hours);
    buf[3] = DecToBcd(time->day_of_week);
    buf[4] = DecToBcd(time->day);
    buf[5] = DecToBcd(time->month);
    buf[6] = DecToBcd(time->year);

    HAL_I2C_Mem_Write(_ds3231_ui2c, DS3231_ADDR, 0x00, 1, buf, 7, 1000);
}

void DS3231_GetTime(RTC_Time_t *time) {
    uint8_t buf[7];

    HAL_I2C_Mem_Read(_ds3231_ui2c, DS3231_ADDR, 0x00, 1, buf, 7, 1000);

    time->seconds = BcdToDec(buf[0]);
    time->minutes = BcdToDec(buf[1]);
    time->hours   = BcdToDec(buf[2]);
    time->day_of_week = BcdToDec(buf[3]);
    time->day     = BcdToDec(buf[4]);
    time->month   = BcdToDec(buf[5]);
    time->year    = BcdToDec(buf[6]);
}
