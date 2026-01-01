#ifndef DIMMER_H
#define DIMMER_H

#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim1;
#define DIMMER_TIM      &htim1
#define DIMMER_CHANNEL  TIM_CHANNEL_1

#define TRIAC_PULSE_WIDTH 20

#define MIN_DELAY 500
#define MAX_DELAY 9500

void Dimmer_Init(void);
void Dimmer_SetValue(uint8_t value);

#endif
