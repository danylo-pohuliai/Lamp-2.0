#include "music.h"
#include "app_state.h"
#include "melodies.h"
#include "cmsis_os.h"
#include <stdlib.h>

static uint8_t current_volume = 100;
static uint8_t current_melody_index = 0;

void Music_Init(void) {
	HAL_TIM_PWM_Stop(BUZZER_TIM, BUZZER_CHANNEL);
}

void Music_Stop(void) {
	HAL_TIM_PWM_Stop(BUZZER_TIM, BUZZER_CHANNEL);
}

void Music_SetVolume(uint8_t percent) {
	if (percent > 100)
		percent = 100;
	current_volume = percent;
}

uint8_t Music_GetVolume(void) {
	return current_volume;
}

int Music_GetMelodyCount(void) {
	return MELODIES_COUNT;
}

const char* Music_GetMelodyName(uint8_t index) {
	if (index < MELODIES_COUNT) {
		return melody_list[index].name;
	}
	return "Unknown";
}

void Music_SelectMelody(uint8_t index) {
	if (index < MELODIES_COUNT) {
		current_melody_index = index;
	}
}

uint8_t Music_GetCurrentMelodyIndex(void) {
	return current_melody_index;
}

void Music_SelectRandomFromPlaylist(void) {
	int total_melodies = Music_GetMelodyCount();
	uint8_t active_indices[MAX_MELODIES_IN_MASK];
	int active_count = 0;

	for (int i = 0; i < total_melodies; i++) {
		if (AppState.melody_playlist[i] == true) {
			active_indices[active_count] = i;
			active_count++;
		}
	}

	if (active_count == 0) {
		Music_SelectMelody(0);
		return;
	}

	uint32_t seed = AppState.now.seconds + HAL_GetTick();
	srand(seed);

	int random_pick = rand() % active_count;
	uint8_t selected_melody_id = active_indices[random_pick];
	Music_SelectMelody(selected_melody_id);
}

static void PlayTone(uint16_t freq) {
	if (freq == 0 || current_volume == 0) {
		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, 0);
	} else {
		uint32_t arr = 1000000 / freq;
		uint32_t ccr = (arr * current_volume) / 200;
		__HAL_TIM_SET_AUTORELOAD(BUZZER_TIM, arr);
		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, ccr);
	}
}

void Music_PlayAlarmLoop(bool use_fade_in) {
	HAL_TIM_PWM_Start(BUZZER_TIM, BUZZER_CHANNEL);
	uint8_t start_melody_index = current_melody_index;
	const Melody_t *melody = &melody_list[start_melody_index];
	int wholenote = (60000 * 4) / melody->tempo;
	int notes = melody->length / 2;
	static uint32_t alarm_start_tick = 0;

	if (alarm_start_tick == 0) {
		alarm_start_tick = HAL_GetTick();
	}

	for (int i = 0; i < notes; i++) {
		if (!AppState.is_alarm_ringing) {
			Music_Stop();
			alarm_start_tick = 0;
			return;
		}
		if (current_melody_index != start_melody_index) {
			Music_Stop();
			alarm_start_tick = 0;
			return;
		}
		if (use_fade_in) {
			uint32_t elapsed_ms = HAL_GetTick() - alarm_start_tick;
			if (elapsed_ms < 4000) {
				current_volume = 10;
			} else if (elapsed_ms < 8000) {
				current_volume = 20;
			} else if (elapsed_ms < 12000) {
				current_volume = 35;
			} else {
				current_volume = 100;
			}
		} else {
			current_volume = AppState.volume;
		}

		uint16_t note = melody->data[i * 2];
		int16_t divider = melody->data[i * 2 + 1];

		int noteDuration = 0;
		if (divider > 0) {
			noteDuration = wholenote / divider;
		} else if (divider < 0) {
			noteDuration = wholenote / abs(divider);
			noteDuration *= 1.5;
		}

		PlayTone(note);
		osDelay(noteDuration * 0.9);

		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, 0);
		osDelay(noteDuration * 0.1);
	}

	Music_Stop();

	for (int k = 0; k < 10; k++) {
		if (!AppState.is_alarm_ringing) {
			alarm_start_tick = 0;
			return;
		}
		osDelay(100);
	}

	alarm_start_tick = 0;
}
