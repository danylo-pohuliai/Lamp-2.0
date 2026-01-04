#ifndef MUSIC_H
#define MUSIC_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

extern TIM_HandleTypeDef htim4;
#define BUZZER_TIM     &htim4
#define BUZZER_CHANNEL TIM_CHANNEL_3

typedef struct {
	const char *name;
	const int16_t *data;
	uint16_t length;
	uint16_t tempo;
} Melody_t;

void Music_Init(void);
void Music_PlayAlarmLoop(bool use_fade_in);
void Music_Stop(void);
void Music_SetVolume(uint8_t percent);
uint8_t Music_GetVolume(void);
void Music_SelectMelody(uint8_t index);
int Music_GetMelodyCount(void);
const char* Music_GetMelodyName(uint8_t index);

#endif
