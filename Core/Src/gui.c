#include "gui.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "app_state.h"
#include <stdio.h>
#include <string.h>

Screen_t CurrentScreen = {0};

static char str_buf[32];

const char* menu_items[] = {"Time", "Alarms", "Set Time", "Set Date", "Light", "Sleep", "Reset"};
#define MENU_COUNT 7
#define MENU_ROW_HEIGHT 10
#define MENU_START_Y    12
#define MENU_COL1_X     12
#define MENU_COL2_X     76
#define MENU_ITEMS_PER_COL 5
#define ALARMS_VISIBLE_ROWS 4
static int8_t menu_cursor = 0, alarms_cursor = 0, edit_pos = 0, list_offset = 0;

static void Helper_GetNextAlarmStr(char* buffer) {
    int32_t current_total_mins = AppState.hour * 60 + AppState.min;
    int32_t min_diff = 99999;
    bool found = false;

    for (int i = 0; i < AppState.alarms_count; i++) {
        if (AppState.alarms[i].active) {
            int32_t alarm_total_mins = AppState.alarms[i].hours * 60 + AppState.alarms[i].mins;

            int32_t diff = alarm_total_mins - current_total_mins;

            if (diff <= 0) {
                diff += 1440;
            }

            if (diff < min_diff) {
                min_diff = diff;
                found = true;
            }
        }
    }

    if (found) {
        sprintf(buffer, "Next: %ldh %ldm", min_diff / 60, min_diff % 60);
    } else {
        strcpy(buffer, "No Alarms");
    }
}

static uint8_t Helper_GetCenterX(const char* text, SSD1306_Font_t font) {
    uint8_t str_len = strlen(text);
    uint8_t pixel_width = str_len * font.width;
    if (pixel_width > 128) return 0;
    return (128 - pixel_width) / 2;
}

void GUI_GoToScreen_Main(void);
void GUI_GoToScreen_Menu(void);
void GUI_GoToScreen_Alarms(void);
void GUI_GoToScreen_SetAlarm(void);
void GUI_GoToScreen_SetTime(void);
void GUI_GoToScreen_SetDate(void);
void GUI_GoToScreen_Light(void);
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
void Screen_Sleep_Draw(void);
void Screen_Sleep_Input(int8_t enc, uint8_t btn);

void GUI_Init(void) {
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
    GUI_GoToScreen_Main();
}

void GUI_Update(void) {
    ssd1306_Fill(Black);
    if (CurrentScreen.draw != NULL) {
        CurrentScreen.draw();
    }

    ssd1306_UpdateScreen();
}

void GUI_HandleInput(int8_t enc, uint8_t btn) {
    if (CurrentScreen.input != NULL) {
        CurrentScreen.input(enc, btn);
    }
}

void GUI_GoToScreen_Main(void) {
    CurrentScreen.draw = Screen_Main_Draw;
    CurrentScreen.input = Screen_Main_Input;
    CurrentScreen.name = "Main";
}

void GUI_GoToScreen_Menu(void) {
	edit_pos = 0;
    CurrentScreen.draw = Screen_Menu_Draw;
    CurrentScreen.input = Screen_Menu_Input;
    CurrentScreen.name = "Menu";
}

void GUI_GoToScreen_Alarms(void) {
	edit_pos = 0;
	CurrentScreen.draw = Screen_Alarms_Draw;
	CurrentScreen.input = Screen_Alarms_Input;
	CurrentScreen.name = "Alarms";
}

void GUI_GoToScreen_SetAlarm(void) {
	edit_pos = 0;
	CurrentScreen.draw = Screen_SetAlarm_Draw;
	CurrentScreen.input = Screen_SetAlarm_Input;
	CurrentScreen.name = "SetAlarm";
}

void GUI_GoToScreen_SetTime(void) {
    edit_pos = 0;
    CurrentScreen.draw = Screen_SetTime_Draw;
    CurrentScreen.input = Screen_SetTime_Input;
    CurrentScreen.name = "SetTime";
}

void GUI_GoToScreen_SetDate(void) {
	edit_pos = 0;
	CurrentScreen.draw = Screen_SetDate_Draw;
	CurrentScreen.input = Screen_SetDate_Input;
	CurrentScreen.name = "SetDate";
}

void GUI_GoToScreen_Light(void) {
	edit_pos = 0;
	CurrentScreen.draw = Screen_Light_Draw;
	CurrentScreen.input = Screen_Light_Input;
	CurrentScreen.name = "Light";
}

void GUI_GoToScreen_Sleep(void) {
	edit_pos = 0;
	CurrentScreen.draw = Screen_Sleep_Draw;
	CurrentScreen.input = Screen_Sleep_Input;
	CurrentScreen.name = "Sleep";
}

void Screen_Main_Draw(void) {
    static uint32_t last_alarm_update = 0;
    static char alarm_cache_buf[32] = "Wait...";

    if (HAL_GetTick() - last_alarm_update >= 1000) {
        last_alarm_update = HAL_GetTick();
        Helper_GetNextAlarmStr(alarm_cache_buf);
    }

    sprintf(str_buf, "Alarms: %d", AppState.alarms_count);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(str_buf, Font_6x8, White);

    sprintf(str_buf, "Bright: %d%%", AppState.brightness);
    ssd1306_SetCursor(0, 9);
    ssd1306_WriteString(str_buf, Font_6x8, White);

    sprintf(str_buf, "%02d:%02d:%02d", AppState.hour, AppState.min, AppState.sec);
    uint8_t time_x = Helper_GetCenterX(str_buf, Font_11x18);
    ssd1306_SetCursor(time_x, 22);
    ssd1306_WriteString(str_buf, Font_11x18, White);

    sprintf(str_buf, "%02d.%02d.20%02d", AppState.day, AppState.month, AppState.year);
    uint8_t date_x = Helper_GetCenterX(str_buf, Font_7x10);
    ssd1306_SetCursor(date_x, 42);
    ssd1306_WriteString(str_buf, Font_7x10, White);

    uint8_t alarm_x = Helper_GetCenterX(alarm_cache_buf, Font_6x8);
    ssd1306_SetCursor(alarm_x, 56);
    ssd1306_WriteString(alarm_cache_buf, Font_6x8, White);
}

void Screen_Main_Input(int8_t enc, uint8_t btn) {
    if (enc != 0) {
        int16_t temp = AppState.brightness + (enc * 5);
        if (temp > 100) temp = 100;
        if (temp < 0) temp = 0;
        AppState.brightness = (uint8_t)temp;
    }

    if (btn == 1) {
        GUI_GoToScreen_Menu();
    }
}

void Screen_Menu_Draw(void) {
    const char* title = "--- MENU ---";
    uint8_t title_x = Helper_GetCenterX(title, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString((char*)title, Font_6x8, White);

    for(int i = 0; i < MENU_COUNT; i++) {
        uint8_t x_pos, y_pos;

        if (i < MENU_ITEMS_PER_COL) {
            x_pos = MENU_COL1_X;
            y_pos = MENU_START_Y + (i * MENU_ROW_HEIGHT);
        } else {
            x_pos = MENU_COL2_X;
            y_pos = MENU_START_Y + ((i - MENU_ITEMS_PER_COL) * MENU_ROW_HEIGHT);
        }

        if (i == menu_cursor) {
            ssd1306_SetCursor(x_pos - 10, y_pos);
            ssd1306_WriteString(">", Font_6x8, White);
        }

        ssd1306_SetCursor(x_pos, y_pos);
        ssd1306_WriteString((char*)menu_items[i], Font_6x8, White);
    }
}

void Screen_Menu_Input(int8_t enc, uint8_t btn) {
    if (enc != 0) {
        menu_cursor += enc;

        if (menu_cursor < 0) menu_cursor = MENU_COUNT - 1;
        if (menu_cursor >= MENU_COUNT) menu_cursor = 0;
    }

    if (btn == 1) {
        switch(menu_cursor) {
            case 0: /* Time */      GUI_GoToScreen_Main(); break;
            case 1: /* Alarms */    GUI_GoToScreen_Alarms(); break;
            case 2: /* Set Time */  GUI_GoToScreen_SetTime(); break;
            case 3: /* Set Date */  GUI_GoToScreen_SetDate(); break;
            case 4: /* Light */     GUI_GoToScreen_Light(); break;
            case 5: /* Sleep */     GUI_GoToScreen_Sleep(); break;
            case 6: /* Reset */     HAL_NVIC_SystemReset(); break;
            default: break;
        }
    }
}

void Screen_Alarms_Draw(void) {
    const char* title = "--- ALARMS ---";
    uint8_t title_x = Helper_GetCenterX(title, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString((char*)title, Font_6x8, White);

    int total_items = AppState.alarms_count + 1;

    for (int i = 0; i < ALARMS_VISIBLE_ROWS; i++) {
        int item_index = list_offset + i;

        if (item_index >= total_items) break;

        uint8_t y_pos = 12 + (i * 12);

        if (item_index == alarms_cursor) {
            ssd1306_SetCursor(0, y_pos);
            ssd1306_WriteString(">", Font_7x10, White);
        }

        ssd1306_SetCursor(10, y_pos);

        if (item_index < AppState.alarms_count) {
            volatile Alarm_t* alm = &AppState.alarms[item_index];

            sprintf(str_buf, "Alarm %d: %02d:%02d %s",
                    item_index,
                    (int)alm->hours,
                    (int)alm->mins,
                    alm->active ? "ON" : "OFF");

            ssd1306_WriteString(str_buf, Font_6x8, White);
        }
        else {
            ssd1306_WriteString("Back", Font_7x10, White);
        }
    }

    if (list_offset > 0) {
        ssd1306_SetCursor(120, 12);
        ssd1306_WriteString("^", Font_6x8, White);
    }

    if (list_offset + ALARMS_VISIBLE_ROWS < total_items) {
        ssd1306_SetCursor(120, 48);
        ssd1306_WriteString("v", Font_6x8, White);
    }
}

void Screen_Alarms_Input(int8_t enc, uint8_t btn) {
    int total_items = AppState.alarms_count + 1;

    if (enc != 0) {
        alarms_cursor += enc;

        if (alarms_cursor < 0) alarms_cursor = total_items - 1;
        if (alarms_cursor >= total_items) alarms_cursor = 0;

        if (alarms_cursor >= list_offset + ALARMS_VISIBLE_ROWS) {
            list_offset = alarms_cursor - ALARMS_VISIBLE_ROWS + 1;
        }

        if (alarms_cursor < list_offset) {
            list_offset = alarms_cursor;
        }

        if (alarms_cursor == total_items - 1 && list_offset == 0) {
             list_offset = total_items - ALARMS_VISIBLE_ROWS;
             if (list_offset < 0) list_offset = 0;
        }
    }

    if (btn == 1) {
        if (alarms_cursor < AppState.alarms_count) {
            GUI_GoToScreen_SetAlarm();
        } else {
            GUI_GoToScreen_Menu();
        }
    }
}

void Screen_SetAlarm_Draw(void) {
	volatile Alarm_t* alm = &AppState.alarms[alarms_cursor];

    sprintf(str_buf, "--- Set Alarm %d ---", alarms_cursor);
    uint8_t title_x = Helper_GetCenterX(str_buf, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString(str_buf, Font_6x8, White);

    if (edit_pos == 0) {
        ssd1306_SetCursor(0, 12);
        ssd1306_WriteString(">", Font_7x10, White);
    }

    ssd1306_SetCursor(10, 12);
    ssd1306_WriteString("Alarm is ", Font_7x10, White);
    if (alm->active) {
        ssd1306_WriteString("ON", Font_7x10, White);
    } else {
        ssd1306_WriteString("OFF", Font_7x10, White);
    }

    const char* label = "Set Alarm Time:";
    uint8_t label_x = Helper_GetCenterX(label, Font_6x8);
    ssd1306_SetCursor(label_x, 30);
    ssd1306_WriteString((char*)label, Font_6x8, White);

    sprintf(str_buf, "%02d : %02d", (int)alm->hours, (int)alm->mins);

    uint8_t time_x = Helper_GetCenterX(str_buf, Font_11x18);
    uint8_t time_y = 42;
    ssd1306_SetCursor(time_x, time_y);
    ssd1306_WriteString(str_buf, Font_11x18, White);

    if (edit_pos == 1) {
        uint8_t x_start = time_x;
        uint8_t x_end = time_x + 22;
        ssd1306_Line(x_start, time_y + 18, x_end, time_y + 18, White);
    }

    if (edit_pos == 2) {
        uint8_t x_start = time_x + (11 * 5);
        uint8_t x_end = x_start + 22;
        ssd1306_Line(x_start, time_y + 18, x_end, time_y + 18, White);
    }
}

void Screen_SetAlarm_Input(int8_t enc, uint8_t btn) {
    volatile Alarm_t* alm = &AppState.alarms[alarms_cursor];

    if (enc != 0) {
        switch (edit_pos) {
            case 0:
                alm->active = !alm->active;
                break;

            case 1:
                {
                    int8_t h = (int)alm->hours + enc;
                    if (h > 23) h = 0;
                    if (h < 0) h = 23;
                    alm->hours = h;
                }
                break;

            case 2:
                {
                    int8_t m = (int)alm->mins + enc;
                    if (m > 59) m = 0;
                    if (m < 0) m = 59;
                    alm->mins = m;
                }
                break;
        }
    }

    if (btn == 1) {
    	edit_pos++;
        if (edit_pos > 2) edit_pos = 0;
    }

    if (btn == 2) {
        // EEPROM TODO
        GUI_GoToScreen_Alarms();
    }
}

void Screen_SetTime_Draw(void) {
    const char* title = "--- Set Time ---";
    uint8_t title_x = Helper_GetCenterX(title, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString((char*)title, Font_6x8, White);

    sprintf(str_buf, "%02d : %02d : %02d", AppState.hour, AppState.min, AppState.sec);

    uint8_t time_x = Helper_GetCenterX(str_buf, Font_11x18);
    uint8_t time_y = 23;

    ssd1306_SetCursor(time_x, time_y);
    ssd1306_WriteString(str_buf, Font_11x18, White);

    uint8_t line_y = time_y + 20;

    if (edit_pos == 0) {
        ssd1306_Line(time_x, line_y, time_x + 22, line_y, White);
    }
    else if (edit_pos == 1) {
        uint8_t min_start_x = time_x + 55;
        ssd1306_Line(min_start_x, line_y, min_start_x + 22, line_y, White);
    }
    else if (edit_pos == 2) {
		uint8_t sec_start_x = time_x + 110;
		ssd1306_Line(sec_start_x, line_y, sec_start_x + 22, line_y, White);
	}
}

void Screen_SetTime_Input(int8_t enc, uint8_t btn) {
    if (enc != 0) {
        if (edit_pos == 0) {
            int8_t h = AppState.hour + enc;
            if (h > 23) h = 0;
            if (h < 0) h = 23;
            AppState.hour = h;
        }
        else if (edit_pos == 1) {
            int8_t m = AppState.min + enc;
            if (m > 59) m = 0;
            if (m < 0) m = 59;
            AppState.min = m;
        }
        else if (edit_pos == 2) {
			int8_t s = AppState.sec + enc;
			if (s > 59) s = 0;
			if (s < 0) s = 59;
			AppState.sec = s;
		}
    }

    if (btn == 1) {
        edit_pos++;
        if (edit_pos > 2) edit_pos = 0;
    }

    if (btn == 2) {
        // RTC_SetTime(AppState.hour, AppState.min, 0); TODO

        GUI_GoToScreen_Menu();
    }
}

void Screen_SetDate_Draw(void) {
    const char* title = "--- Set Date ---";
    uint8_t title_x = Helper_GetCenterX(title, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString((char*)title, Font_6x8, White);

    sprintf(str_buf, "%02d . %02d . %02d", AppState.day, AppState.month, AppState.year);

    uint8_t date_x = Helper_GetCenterX(str_buf, Font_11x18);
    uint8_t date_y = 23;

    ssd1306_SetCursor(date_x, date_y);
    ssd1306_WriteString(str_buf, Font_11x18, White);

    uint8_t line_y = date_y + 20;

    if (edit_pos == 0) {
        ssd1306_Line(date_x, line_y, date_x + 22, line_y, White);
    }
    else if (edit_pos == 1) {
        uint8_t m_start = date_x + 55;
        ssd1306_Line(m_start, line_y, m_start + 22, line_y, White);
    }
    else if (edit_pos == 2) {
        uint8_t y_start = date_x + 110;
        ssd1306_Line(y_start, line_y, y_start + 22, line_y, White);
    }
}

void Screen_SetDate_Input(int8_t enc, uint8_t btn) {
    if (enc != 0) {
        switch (edit_pos) {
            case 0:
                {
                    int8_t d = AppState.day + enc;
                    if (d > 31) d = 1;
                    if (d < 1) d = 31;
                    AppState.day = d;
                }
                break;

            case 1:
                {
                    int8_t m = AppState.month + enc;
                    if (m > 12) m = 1;
                    if (m < 1) m = 12;
                    AppState.month = m;
                }
                break;

            case 2:
                {
                    int8_t y = AppState.year + enc;
                    if (y > 99) y = 24;
                    if (y < 24) y = 99;
                    AppState.year = y;
                }
                break;
        }
    }

    if (btn == 1) {
        edit_pos++;
        if (edit_pos > 2) edit_pos = 0;
    }

    if (btn == 2) {
        GUI_GoToScreen_Menu();
    }
}

void Screen_Light_Draw(void) {
    const char* title = "--- Light ---";
    uint8_t title_x = Helper_GetCenterX(title, Font_6x8);
    ssd1306_SetCursor(title_x, 0);
    ssd1306_WriteString((char*)title, Font_6x8, White);

    if (edit_pos == 0) {
        ssd1306_SetCursor(0, 15);
        ssd1306_WriteString(">", Font_7x10, White);
    }
    ssd1306_SetCursor(12, 15);
    ssd1306_WriteString("Turn ON", Font_7x10, White);

    if (edit_pos == 1) {
        ssd1306_SetCursor(0, 28);
        ssd1306_WriteString(">", Font_7x10, White);
    }
    ssd1306_SetCursor(12, 28);
    ssd1306_WriteString("Turn OFF", Font_7x10, White);

    const char* br_label = "Brightness:";
    uint8_t br_x = Helper_GetCenterX(br_label, Font_6x8);
    ssd1306_SetCursor(br_x, 42);
    ssd1306_WriteString((char*)br_label, Font_6x8, White);

    sprintf(str_buf, "%d%%", AppState.brightness);
    uint8_t val_x = Helper_GetCenterX(str_buf, Font_11x18);
    uint8_t val_y = 52;

    if (edit_pos == 2) {
        ssd1306_SetCursor(val_x - 15, val_y);
        ssd1306_WriteString("<", Font_11x18, White);

        ssd1306_SetCursor(val_x + 35, val_y);
        ssd1306_WriteString(">", Font_11x18, White);
    }

    ssd1306_SetCursor(val_x, val_y);
    ssd1306_WriteString(str_buf, Font_11x18, White);
}

void Screen_Light_Input(int8_t enc, uint8_t btn) {
    if (edit_pos == 2) {

        if (enc != 0) {
            int16_t val = AppState.brightness + (enc * 5);
            if (val > 100) val = 100;
            if (val < 0) val = 0;
            AppState.brightness = (uint8_t)val;
            if (val > 0) AppState.is_light_on = true;
        }

        if (btn == 1) {
        	edit_pos = 0;
        }
    }

    else {
        if (enc != 0) {
        	edit_pos += enc;

            if (edit_pos > 2) edit_pos = 0;
            if (edit_pos < 0) edit_pos = 2;
        }

        if (btn == 1) {
            if (edit_pos == 0) {
                AppState.brightness = 100;
                AppState.is_light_on = true;
            }
            if (edit_pos == 1) {
                AppState.brightness = 0;
                AppState.is_light_on = false;
            }
        }
    }

    if (btn == 2) {
        GUI_GoToScreen_Menu();
    }
}

void Screen_Sleep_Draw(void) {
    // NOTHING
}

void Screen_Sleep_Input(int8_t enc, uint8_t btn) {
    if (enc != 0 || btn != 0) {
        GUI_GoToScreen_Main();
    }
}
