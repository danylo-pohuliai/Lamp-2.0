#include "music.h"
#include "app_state.h"
#include "cmsis_os.h"

#define TEMPO_MULTIPLIER 1.0 
#define N_16 110
#define N_8  220

const Note_t doom_melody[] = { { NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_E3,
N_16 }, { NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_D3, N_16 }, {
NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_C3, N_16 }, { NOTE_REST, N_16 }, {
NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_AS2, N_16 }, { NOTE_E2,
N_16 }, { NOTE_E2, N_16 }, { NOTE_B2, N_16 }, { NOTE_C3, N_16 },
		{ NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_C3, N_16 },
		{ NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_E3, N_16 },
		{ NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_D3, N_16 },
		{ NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_C3, N_16 }, { NOTE_REST,
		N_16 }, { NOTE_E2, N_16 }, { NOTE_E2, N_16 }, { NOTE_AS2, N_16 }, {
		NOTE_REST, 300 } };

#define MELODY_LENGTH (sizeof(doom_melody) / sizeof(Note_t))

void Music_Init(void) {
	HAL_TIM_PWM_Stop(BUZZER_TIM, BUZZER_CHANNEL);
}

void Music_Stop(void) {
	HAL_TIM_PWM_Stop(BUZZER_TIM, BUZZER_CHANNEL);
}

static void PlayTone(uint16_t freq) {
	if (freq == 0) {
		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, 0);
	} else {
		uint32_t arr = 1000000 / freq;

		__HAL_TIM_SET_AUTORELOAD(BUZZER_TIM, arr);
		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, arr / 2);
	}
}

void Music_PlayAlarmLoop(void) {
	HAL_TIM_PWM_Start(BUZZER_TIM, BUZZER_CHANNEL);

	for (int i = 0; i < MELODY_LENGTH; i++) {
		if (!AppState.is_alarm_ringing) {
			Music_Stop();
			return;
		}

		PlayTone(doom_melody[i].frequency);
		osDelay(doom_melody[i].duration_ms);

		__HAL_TIM_SET_COMPARE(BUZZER_TIM, BUZZER_CHANNEL, 0);
		osDelay(20);
	}
}
