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

void GUI_GoToScreen_Main(void);
void GUI_GoToScreen_Menu(void);
void GUI_GoToScreen_Alarms(void);
void GUI_GoToScreen_SetAlarm(void);
void GUI_GoToScreen_SetTime(void);
void GUI_GoToScreen_SetDate(void);
void GUI_GoToScreen_Light(void);
void GUI_GoToScreen_Music(void);
void GUI_GoToScreen_MelodySettings(void);
void GUI_GoToScreen_Sleep(void);

void Screen_Main_Draw(void);
void Screen_Main_Input(int8_t enc, uint8_t btn);
void Screen_Menu_Draw(void);
void Screen_Menu_Input(int8_t enc, uint8_t btn);
void Screen_Alarms_Draw(void);
void Screen_Alarms_Input(int8_t enc, uint8_t btn);
void Screen_SetAlarm_Draw(void);
void Screen_SetAlarm_Input(int8_t enc, uint8_t btn);
void Screen_SetTime_Draw(void);
void Screen_SetTime_Input(int8_t enc, uint8_t btn);
void Screen_SetDate_Draw(void);
void Screen_SetDate_Input(int8_t enc, uint8_t btn);
void Screen_Light_Draw(void);
void Screen_Light_Input(int8_t enc, uint8_t btn);
void Screen_Music_Draw(void);
void Screen_Music_Input(int8_t enc, uint8_t btn);
void Screen_AlarmPlaylist_Draw(void);
void Screen_AlarmPlaylist_Input(int8_t enc, uint8_t btn);
void Screen_MelodySettings_Draw(void);
void Screen_MelodySettings_Input(int8_t enc, uint8_t btn);
void Screen_Sleep_Draw(void);
void Screen_Sleep_Input(int8_t enc, uint8_t btn);

#endif
