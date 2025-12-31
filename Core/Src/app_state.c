/*
 * app_state.c
 * Реалізація глобального стану
 */

#include "app_state.h"

// Фізичне створення змінної.
// Тільки тут ми не пишемо "extern".
volatile SystemState_t AppState = {
    // Початкові значення (Defaults)

    // Час (поки не зчитали RTC, буде 12:00)
    .hour = 12,
    .min = 0,
    .sec = 0,

    .day = 1,
    .month = 1,
    .year = 26,

    // Батарея
    .battery_voltage = 0.0f,

    // Світло
    .brightness = 0,    // Старт з 50% яскравості
    .is_light_on = false, // Лампа увімкнена при старті

    // Будильники
    .alarms_count = 0,
    .is_alarm_ringing = false
};
