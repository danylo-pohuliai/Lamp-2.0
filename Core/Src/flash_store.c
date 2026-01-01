#include "flash_store.h"
#include <string.h>

#define FLASH_START_ADDR 0x0801FC00
#ifndef FLASH_PAGE_SIZE
  #define FLASH_PAGE_SIZE  1024
#endif
#define FLASH_END_ADDR   (FLASH_START_ADDR + FLASH_PAGE_SIZE)
#define SETTINGS_SIZE    (sizeof(Settings_t))
#define MAGIC_NUMBER     0xA5

extern volatile SystemState_t AppState;

static uint32_t Find_Empty_Slot(void) {
    uint32_t addr = FLASH_START_ADDR;

    while (addr < FLASH_END_ADDR - SETTINGS_SIZE) {
        Settings_t* ptr = (Settings_t*)addr;

        if (ptr->magic_num == 0xFF) {
            return addr;
        }

        addr += SETTINGS_SIZE;
    }

    return 0;
}

static uint32_t Find_Last_Valid_Slot(void) {
    uint32_t addr = FLASH_START_ADDR;
    uint32_t last_valid_addr = 0;

    while (addr < FLASH_END_ADDR - SETTINGS_SIZE) {
        Settings_t* ptr = (Settings_t*)addr;

        if (ptr->magic_num == MAGIC_NUMBER) {
            last_valid_addr = addr;
        } else {
            break;
        }
        addr += SETTINGS_SIZE;
    }

    return last_valid_addr;
}

void Flash_SaveSettings(void) {
    Settings_t data;
    data.magic_num = MAGIC_NUMBER;
    data.alarms_count = AppState.alarms_count;
    data.brightness = AppState.brightness;
    data.is_light_on = (uint8_t)AppState.is_light_on;
    memcpy(data.alarms, (void*)AppState.alarms, sizeof(AppState.alarms));

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

    uint32_t *source_addr = (uint32_t *)&data;
    int words = (sizeof(Settings_t) + 3) / 4;

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
        Settings_t* saved = (Settings_t*)valid_addr;

        AppState.alarms_count = saved->alarms_count;
        if (AppState.alarms_count > MAX_ALARMS) AppState.alarms_count = 0;

        AppState.brightness = saved->brightness;
        AppState.is_light_on = (bool)saved->is_light_on;

        memcpy((void*)AppState.alarms, saved->alarms, sizeof(saved->alarms));
    } else {
        AppState.alarms_count = 0;
    }
}
