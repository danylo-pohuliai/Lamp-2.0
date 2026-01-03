#include "app_state.h"
#include "flash_store.h"
#include "dimmer.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>

volatile SystemState_t AppState = { .now = { .hours = 12, .minutes = 0,
		.seconds = 0, .day = 1, .month = 1, .year = 26, .day_of_week = 4 },
		.battery_voltage = 0.0f, .brightness = 0, .is_light_on = false,

		.alarms_count = 0, .is_alarm_ringing = false };

static const char *week_days[] = { "Err", "Mon", "Tue", "Wed", "Thu", "Fri",
		"Sat", "Sun" };

void AppState_Init(void) {
	Flash_LoadSettings();
}

void AppState_AddAlarm(uint8_t h, uint8_t m) {
	if (AppState.alarms_count >= MAX_ALARMS) {
		return;
	}

	int idx = AppState.alarms_count;
	AppState.alarms[idx].hours = h;
	AppState.alarms[idx].mins = m;
	AppState.alarms[idx].active = true;

	AppState.alarms_count++;

	Flash_SaveSettings();
}

void AppState_DeleteAlarm(uint8_t id) {
	if (id >= AppState.alarms_count) {
		return;
	}

	for (int i = id; i < AppState.alarms_count - 1; i++) {
		AppState.alarms[i] = AppState.alarms[i + 1];
	}

	AppState.alarms_count--;
	AppState.alarms[AppState.alarms_count].active = false;
	AppState.alarms[AppState.alarms_count].hours = 0;
	AppState.alarms[AppState.alarms_count].mins = 0;

	Flash_SaveSettings();
}

bool AppState_ToggleAlarm(uint8_t id) {
	if (id >= AppState.alarms_count)
		return false;
	AppState.alarms[id].active = !AppState.alarms[id].active;
	Flash_SaveSettings();

	return AppState.alarms[id].active;
}

void AppState_EditAlarm(uint8_t id, uint8_t h, uint8_t m) {
	if (id >= AppState.alarms_count)
		return;
	AppState.alarms[id].hours = h;
	AppState.alarms[id].mins = m;
	AppState.alarms[id].active = true;

	Flash_SaveSettings();
}

void AppState_GetNextAlarmString(char *buffer) {
	int32_t current_total_mins = AppState.now.hours * 60 + AppState.now.minutes;
	int32_t min_diff = 99999;
	bool found = false;

	for (int i = 0; i < AppState.alarms_count; i++) {
		if (AppState.alarms[i].active) {
			int32_t alarm_total_mins = AppState.alarms[i].hours * 60
					+ AppState.alarms[i].mins;

			int32_t diff = alarm_total_mins - current_total_mins;
			if (diff <= 0) {
				diff += 1440;
			}

			if (diff < min_diff) {
				min_diff = diff;
				found = true;
			}
		}
	}

	if (found) {
		sprintf(buffer, "Time remaining: %ldh %ldm", min_diff / 60,
				min_diff % 60);
	} else {
		strcpy(buffer, "No active alarms");
	}
}

void AppState_CheckAlarms(void) {
	static uint8_t last_checked_minute = 60;
	if (AppState.now.minutes == last_checked_minute) {
		return;
	}
	last_checked_minute = AppState.now.minutes;

	if (AppState.is_alarm_ringing) {
		return;
	}
	for (int i = 0; i < AppState.alarms_count; i++) {
		if (AppState.alarms[i].active
				&& AppState.alarms[i].hours == AppState.now.hours
				&& AppState.alarms[i].mins == AppState.now.minutes) {

			AppState.is_alarm_ringing = true;
			return;
		}
	}
}

void AppState_SetBrightness(uint8_t new_val) {
	if (new_val > 100)
		new_val = 100;

	AppState.brightness = new_val;
	Dimmer_SetValue(AppState.brightness);
}

const char* AppState_GetDayOfWeekStr(uint8_t day_idx) {
	if (day_idx >= 1 && day_idx <= 7) {
		return week_days[day_idx];
	}
	return week_days[0];
}

void AppState_UpdateBatteryVoltage(void) {
	HAL_ADC_Start(&hadc1);

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
		uint32_t raw = HAL_ADC_GetValue(&hadc1);

		float current_volt = (float) raw * 6.6f / 4095.0f;

		if (AppState.battery_voltage < 0.1f) {
			AppState.battery_voltage = current_volt;
		} else {
			AppState.battery_voltage = (AppState.battery_voltage * 0.9f)
					+ (current_volt * 0.1f);
		}
	}

	HAL_ADC_Stop(&hadc1);
}
