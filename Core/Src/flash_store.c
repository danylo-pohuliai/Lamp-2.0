#include "flash_store.h"
#include "music.h"
#include <string.h>

#define FLASH_START_ADDR 0x0801FC00
#ifndef FLASH_PAGE_SIZE
  #define FLASH_PAGE_SIZE  1024
#endif
#define FLASH_END_ADDR   (FLASH_START_ADDR + FLASH_PAGE_SIZE)
#define SETTINGS_SIZE    (sizeof(Settings_t))
#define MAGIC_NUMBER     0xCAFEBABE

extern volatile SystemState_t AppState;

static uint32_t Find_Empty_Slot(void) {
	uint32_t addr = FLASH_START_ADDR;

	while (addr <= FLASH_END_ADDR - SETTINGS_SIZE) {
		if (*(uint32_t*) addr == 0xFFFFFFFF) {
			return addr;
		}

		addr += (SETTINGS_SIZE + 3) & ~0x03;
	}

	return 0;
}

static uint32_t Find_Last_Valid_Slot(void) {
	uint32_t addr = FLASH_START_ADDR;
	uint32_t last_valid_addr = 0;

	while (addr <= FLASH_END_ADDR - SETTINGS_SIZE) {
		Settings_t *ptr = (Settings_t*) addr;

		if (ptr->magic_num == MAGIC_NUMBER) {
			last_valid_addr = addr;
		} else {
			break;
		}
		addr += (SETTINGS_SIZE + 3) & ~0x03;
	}

	return last_valid_addr;
}

void Flash_SaveSettings(void) {
	Settings_t data;
	memset(&data, 0, sizeof(Settings_t));
	data.magic_num = MAGIC_NUMBER;
	data.alarms_count = AppState.alarms_count;
	data.fade_speed = AppState.fade_speed;
	data.melody_mask = 0;
	int total_melodies = Music_GetMelodyCount();
	if (total_melodies > 32)
		total_melodies = 32;

	for (int i = 0; i < total_melodies; i++) {
		if (AppState.melody_playlist[i]) {
			data.melody_mask |= (1UL << i);
		}
	}

	int alarms_to_copy =
			(AppState.alarms_count > MAX_ALARMS) ?
			MAX_ALARMS :
													AppState.alarms_count;

	for (int i = 0; i < alarms_to_copy; i++) {
		uint8_t hrs = AppState.alarms[i].hours & 0x1F;

		if (AppState.alarms[i].active) {
			hrs |= 0x80;
		}

		data.alarms[i].active_and_hours = hrs;
		data.alarms[i].mins = AppState.alarms[i].mins;
	}

	HAL_FLASH_Unlock();

	uint32_t dest_addr = Find_Empty_Slot();

	if (dest_addr == 0) {
		FLASH_EraseInitTypeDef EraseInitStruct;
		uint32_t PageError;
		EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.PageAddress = FLASH_START_ADDR;
		EraseInitStruct.NbPages = 1;

		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
			HAL_FLASH_Lock();
			return;
		}

		dest_addr = FLASH_START_ADDR;
	}

	uint32_t *source_addr = (uint32_t*) &data;
	int words = sizeof(Settings_t) / 4;

	for (int i = 0; i < words; i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr, *source_addr);
		dest_addr += 4;
		source_addr++;
	}

	HAL_FLASH_Lock();
}

void Flash_LoadSettings(void) {
	uint32_t valid_addr = Find_Last_Valid_Slot();

	if (valid_addr != 0) {
		Settings_t *saved = (Settings_t*) valid_addr;

		AppState.alarms_count = saved->alarms_count;
		if (AppState.alarms_count > MAX_ALARMS)
			AppState.alarms_count = MAX_ALARMS;

		AppState.fade_speed = (saved->fade_speed == 0) ? 1 : saved->fade_speed;
		for (int i = 0; i < AppState.alarms_count; i++) {
			uint8_t packed_val = saved->alarms[i].active_and_hours;

			AppState.alarms[i].hours = packed_val & 0x1F;
			AppState.alarms[i].active = (packed_val & 0x80) != 0;
			AppState.alarms[i].mins = saved->alarms[i].mins;
		}

		int total_melodies = Music_GetMelodyCount();
		if (total_melodies > 32)
			total_melodies = 32;

		for (int i = 0; i < total_melodies; i++) {
			if (saved->melody_mask & (1UL << i)) {
				AppState.melody_playlist[i] = true;
			} else {
				AppState.melody_playlist[i] = false;
			}
		}
	} else {
		AppState.alarms_count = 0;
		AppState.fade_speed = 3;
		int total = Music_GetMelodyCount();
		AppState.melody_playlist[0] = true;
		for (int i = 1; i < total; i++)
			AppState.melody_playlist[i] = false;
	}
}
