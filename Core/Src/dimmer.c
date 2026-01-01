#include "dimmer.h"

void Dimmer_Init(void) {
	HAL_TIM_PWM_Stop(DIMMER_TIM, DIMMER_CHANNEL);
}

void Dimmer_SetValue(uint8_t val) {
	if (val > 100)
		val = 100;

	if (val == 0) {
		HAL_TIM_PWM_Stop(DIMMER_TIM, DIMMER_CHANNEL);
		return;
	}

	uint16_t delay = MAX_DELAY - ((val * (MAX_DELAY - MIN_DELAY)) / 100);

	__HAL_TIM_SET_AUTORELOAD(DIMMER_TIM, delay + TRIAC_PULSE_WIDTH);
	__HAL_TIM_SET_COMPARE(DIMMER_TIM, DIMMER_CHANNEL, delay);
	htim1.Instance->EGR = TIM_EGR_UG;
	__HAL_TIM_CLEAR_FLAG(DIMMER_TIM, TIM_FLAG_UPDATE);

	HAL_TIM_PWM_Start(DIMMER_TIM, DIMMER_CHANNEL);
}
