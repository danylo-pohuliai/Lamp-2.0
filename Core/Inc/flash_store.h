#ifndef FLASH_STORE_H
#define FLASH_STORE_H

#include "stm32f1xx_hal.h"
#include "app_state.h"

typedef struct {
    uint8_t magic_num;     // 0xA5
    uint8_t alarms_count;
    uint8_t brightness;
    uint8_t is_light_on;
    Alarm_t alarms[MAX_ALARMS];
    uint8_t padding;
} Settings_t;

void Flash_SaveSettings(void);
void Flash_LoadSettings(void);

#endif
