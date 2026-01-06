#ifndef FLASH_STORE_H
#define FLASH_STORE_H

#include "stm32f1xx_hal.h"
#include "app_state.h"

typedef struct {
	uint8_t magic_num;
	uint8_t alarms_count;
	uint16_t padding;
	uint32_t melody_mask;
	Alarm_t alarms[MAX_ALARMS];
} Settings_t;

void Flash_SaveSettings(void);
void Flash_LoadSettings(void);

#endif
