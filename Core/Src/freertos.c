/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdbool.h>
#include "tim.h"
#include "gui.h"
#include "app_state.h"
#include "command_parser.h"
#include "bluetooth.h"
#include "ds3231.h"
#include "music.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern TIM_HandleTypeDef htim2;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBOUNCE_DELAY 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint32_t last_counter_value = 0;
static uint32_t last_btn_press_time = 0;
/* USER CODE END Variables */
osThreadId GUITaskHandle;
osThreadId MusicTaskHandle;
osThreadId AlarmTaskHandle;
osThreadId CommTaskHandle;
osThreadId InputTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartGUITask(void const *argument);
void StartMusicTask(void const *argument);
void StartAlarmTask(void const *argument);
void StartCommTask(void const *argument);
void StartInputTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of GUITask */
	osThreadDef(GUITask, StartGUITask, osPriorityNormal, 0, 256);
	GUITaskHandle = osThreadCreate(osThread(GUITask), NULL);

	/* definition and creation of MusicTask */
	osThreadDef(MusicTask, StartMusicTask, osPriorityHigh, 0, 128);
	MusicTaskHandle = osThreadCreate(osThread(MusicTask), NULL);

	/* definition and creation of AlarmTask */
	osThreadDef(AlarmTask, StartAlarmTask, osPriorityNormal, 0, 128);
	AlarmTaskHandle = osThreadCreate(osThread(AlarmTask), NULL);

	/* definition and creation of CommTask */
	osThreadDef(CommTask, StartCommTask, osPriorityLow, 0, 600);
	CommTaskHandle = osThreadCreate(osThread(CommTask), NULL);

	/* definition and creation of InputTask */
	osThreadDef(InputTask, StartInputTask, osPriorityAboveNormal, 0, 128);
	InputTaskHandle = osThreadCreate(osThread(InputTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartGUITask */
/**
 * @brief  Function implementing the GUITask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGUITask */
void StartGUITask(void const *argument) {
	/* USER CODE BEGIN StartGUITask */
	GUI_Init();
	/* Infinite loop */
	for (;;) {
		GUI_Update();
		osDelay(100);
	}
	/* Infinite loop */
	/* USER CODE END StartGUITask */
}

/* USER CODE BEGIN Header_StartMusicTask */
/**
 * @brief Function implementing the MusicTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMusicTask */
void StartMusicTask(void const *argument) {
	/* USER CODE BEGIN StartMusicTask */
	osDelay(500);
	Music_Init();
	/* Infinite loop */
	for (;;) {
		if (AppState.is_alarm_ringing) {
			bool enable_fade = !AppState.is_preview_mode;
			Music_PlayAlarmLoop(enable_fade);
		} else {
			Music_Stop();
			osDelay(200);
		}
	}
	/* USER CODE END StartMusicTask */
}

/* USER CODE BEGIN Header_StartAlarmTask */
/**
 * @brief Function implementing the AlarmTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartAlarmTask */
void StartAlarmTask(void const *argument) {
	/* USER CODE BEGIN StartAlarmTask */
	osDelay(100);
	uint8_t batt_check_counter = 0;
	/* Infinite loop */
	for (;;) {
		DS3231_GetTime((RTC_Time_t*) &AppState.now);
		AppState_CheckAlarms();
		batt_check_counter++;
		if (batt_check_counter >= 5) {
			AppState_UpdateBatteryVoltage();
			batt_check_counter = 0;
		}
		osDelay(200);
	}
	/* USER CODE END StartAlarmTask */
}

/* USER CODE BEGIN Header_StartCommTask */
/**
 * @brief Function implementing the CommTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommTask */
void StartCommTask(void const *argument) {
	/* USER CODE BEGIN StartCommTask */
	Bluetooth_Init();
	bool is_bt_connected = false;

	/* Infinite loop */
	for (;;) {
		if (bt_command_received) {
			bt_command_received = 0;
			CLI_ProcessCommand(bt_command_buffer);
			memset(bt_command_buffer, 0, sizeof(bt_command_buffer));
		}

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET) {

			if (!is_bt_connected) {
				osDelay(100);
				is_bt_connected = true;
				memset(bt_command_buffer, 0, sizeof(bt_command_buffer));
				CLI_ProcessCommand("i");
			}
		} else {
			is_bt_connected = false;
		}
		osDelay(50);
	}
	/* USER CODE END StartCommTask */
}

/* USER CODE BEGIN Header_StartInputTask */
/**
 * @brief Function implementing the InputTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartInputTask */
void StartInputTask(void const *argument) {
	/* USER CODE BEGIN StartInputTask */
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	__HAL_TIM_SET_COUNTER(&htim2, 32000);
	last_counter_value = 32000;
	static uint32_t btn_press_start = 0;
	static bool long_press_handled = false;
	const uint32_t LONG_PRESS_MS = 600;

	for (;;) {
		uint32_t current_counter = __HAL_TIM_GET_COUNTER(&htim2);
		int8_t enc_diff = 0;
		int16_t diff = (int16_t) (current_counter - last_counter_value);

		if (abs(diff) >= 4) {
			if (diff > 0)
				enc_diff = -1;
			else
				enc_diff = 1;

			last_counter_value = current_counter;
		}

		uint8_t btn_event = 0;

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_RESET) {
			if (btn_press_start == 0) {
				btn_press_start = HAL_GetTick();
				long_press_handled = false;
			} else {
				if (!long_press_handled
						&& (HAL_GetTick() - btn_press_start > LONG_PRESS_MS)) {
					btn_event = 5;
					long_press_handled = true;
				}
			}
		} else {
			if (btn_press_start != 0) {
				if (!long_press_handled
						&& (HAL_GetTick() - btn_press_start > DEBOUNCE_DELAY)) {
					btn_event = 1;
				}
				btn_press_start = 0;
				long_press_handled = false;
			}
		}

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
			if (HAL_GetTick() - last_btn_press_time > DEBOUNCE_DELAY) {
				btn_event = 2;
				last_btn_press_time = HAL_GetTick();
				AppState.is_alarm_ringing = false;
				AppState.is_preview_mode = false;
			}
		}

		if (enc_diff != 0 || btn_event != 0) {
			GUI_HandleInput(enc_diff, btn_event);
		}

		osDelay(10);
	}
	/* USER CODE END StartInputTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

