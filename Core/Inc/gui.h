#ifndef GUI_H
#define GUI_H

#include "main.h"

typedef void (*InputCallback)(int8_t encoder_diff, uint8_t btn_state);
typedef void (*DrawCallback)(void);

typedef struct {
	DrawCallback draw;
	InputCallback input;
	const char *name;
} Screen_t;

extern Screen_t CurrentScreen;
extern uint32_t last_activity_time;

void GUI_Init(void);
void GUI_Update(void);
void GUI_HandleInput(int8_t enc, uint8_t btn);

#endif
