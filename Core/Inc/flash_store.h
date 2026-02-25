#ifndef FLASH_STORE_H
#define FLASH_STORE_H

#include "stm32f1xx_hal.h"
#include "app_state.h"

typedef struct __attribute__((packed)) {
	uint8_t active_and_hours;
	uint8_t mins;
} PackedAlarm_t;
// 32 bytes!!!!!!!!!!!!!
typedef struct __attribute__((packed)) {
	uint32_t melody_mask;     // 4
	uint8_t alarms_count;    // 1
	uint8_t fade_speed;      // 1
	uint16_t reserved;      // 2
	PackedAlarm_t alarms[MAX_ALARMS]; // 20
	uint32_t magic_num;       // 4
} Settings_t;

typedef char static_assertion_settings_size[(sizeof(Settings_t) == 32) ? 1 : -1];

void Flash_SaveSettings(void);
void Flash_LoadSettings(void);

#endif
