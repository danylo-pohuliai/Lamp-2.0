#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "ds3231.h"

#define MAX_ALARMS 8
#define MAX_MELODIES_IN_MASK 32

typedef struct {
	uint8_t hours;
	uint8_t mins;
	bool active;
} Alarm_t;

typedef struct {
	RTC_Time_t now;
	Alarm_t alarms[MAX_ALARMS];
	float battery_voltage;
	uint8_t brightness;
	uint8_t alarms_count;
	uint8_t volume;
	bool is_light_on;
	bool is_alarm_ringing;
	bool is_preview_mode;
	bool melody_playlist[MAX_MELODIES_IN_MASK];
} SystemState_t;

extern volatile SystemState_t AppState;

void AppState_Init(void);
void AppState_AddAlarm(uint8_t h, uint8_t m);
void AppState_DeleteAlarm(uint8_t id);
bool AppState_ToggleAlarm(uint8_t id);
void AppState_EditAlarm(uint8_t id, uint8_t h, uint8_t m);
void AppState_GetNextAlarmString(char *buffer);
void AppState_CheckAlarms(void);
void AppState_SetBrightness(uint8_t new_val);
const char* AppState_GetDayOfWeekStr(uint8_t day_idx);
void AppState_UpdateBatteryVoltage(void);

#endif
