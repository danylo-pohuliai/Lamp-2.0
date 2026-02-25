#include "dimmer.h"

void Dimmer_Init(void) {
	HAL_TIM_PWM_Stop(DIMMER_TIM, DIMMER_CHANNEL);
}

void Dimmer_SetValue(uint8_t val) {
	static uint8_t last_val = 255;

	if (val > 100)
		val = 100;

	if (val == last_val)
		return;

	if (val == 0) {
		HAL_TIM_PWM_Stop(DIMMER_TIM, DIMMER_CHANNEL);
		last_val = 0;
		return;
	}

	uint16_t delay = MAX_DELAY - ((val * (MAX_DELAY - MIN_DELAY)) / 100);

	__HAL_TIM_SET_AUTORELOAD(DIMMER_TIM, delay + TRIAC_PULSE_WIDTH);
	__HAL_TIM_SET_COMPARE(DIMMER_TIM, DIMMER_CHANNEL, delay);

	if (last_val == 0) {
		HAL_TIM_PWM_Start(DIMMER_TIM, DIMMER_CHANNEL);
	}

	last_val = val;
}
