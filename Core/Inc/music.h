#ifndef MUSIC_H
#define MUSIC_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

extern TIM_HandleTypeDef htim4;
#define BUZZER_TIM     &htim4
#define BUZZER_CHANNEL TIM_CHANNEL_3

#define NOTE_REST 0
#define NOTE_E2   82
#define NOTE_F2   87
#define NOTE_G2   98
#define NOTE_GS2  104
#define NOTE_A2   110
#define NOTE_AS2  117
#define NOTE_B2   123
#define NOTE_C3   131
#define NOTE_CS3  139
#define NOTE_D3   147
#define NOTE_DS3  156
#define NOTE_E3   165

typedef struct {
	uint16_t frequency;
	uint16_t duration_ms;
} Note_t;

void Music_Init(void);
void Music_PlayAlarmLoop(void);
void Music_Stop(void);

#endif
