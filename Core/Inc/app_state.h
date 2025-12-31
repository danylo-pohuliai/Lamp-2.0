#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ALARMS 9

typedef struct {
    uint8_t hours;
    uint8_t mins;
    bool active;
} Alarm_t;

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;

    uint8_t day;
    uint8_t month;
    uint8_t year;

    float battery_voltage;

    uint8_t brightness;
    bool is_light_on;

    Alarm_t alarms[MAX_ALARMS];
    uint8_t alarms_count;
    bool is_alarm_ringing;

} SystemState_t;

extern volatile SystemState_t AppState;

#endif
