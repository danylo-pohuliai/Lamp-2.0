#include "command_parser.h"
#include "bluetooth.h"
#include "app_state.h"
#include "gui.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char tx_buffer[768];

static void Reply(const char *msg) {
	Bluetooth_Send((char*) msg);
	printf(">> REPLIED: %s", msg);
}

static char Normalize_Command(char *input, int *offset) {
	uint8_t b1 = (uint8_t) input[0];
	uint8_t b2 = (uint8_t) input[1];

	*offset = 1;

	if (b1 < 128) {
		return tolower((char )b1);
	}

	if (b1 == 0xD0 || b1 == 0xD1) {
		*offset = 2;
		if (b2 == 0xB0 || b2 == 0x90)
			return 'a';
		if (b2 == 0xBE || b2 == 0x9E)
			return 'o';
		if (b2 == 0x81 || b2 == 0xA1)
			return 'c';
	}

	return '?';
}

static bool Parse_Time_Flexible(char *args, int8_t *h, int8_t *m) {
	int v1, v2;
	if (sscanf(args, "%d%*c%d", &v1, &v2) == 2) {
		*h = v1;
		*m = v2;
		return true;
	}
	if (sscanf(args, "%d", &v1) == 1) {
		if (v1 >= 100) {
			*h = v1 / 100;
			*m = v1 % 100;
		} else {
			*h = v1;
			*m = 0;
		}
		return true;
	}
	return false;
}

static void Cmd_SetBrightness(char *args) {
	int val;
	if (sscanf(args, "%d", &val) == 1) {
		if (val > 100)
			val = 100;
		if (val < 0)
			val = 0;
		AppState_SetBrightness(val);
		if (val > 0)
			AppState.is_light_on = true;

		char buf[32];
		sprintf(buf, "OK: Brightness %d%%\r\n", val);
		Reply(buf);
	} else {
		Reply("ERR: Invalid Number\r\n");
	}
}

static void Cmd_SetAlarm(char *args) {
	if (*args == '\0') {
		int len = 0;
		len += sprintf(tx_buffer + len, "--- ALARMS LIST ---\r\n");

		if (AppState.alarms_count == 0) {
			len += sprintf(tx_buffer + len, "No alarms set\r\n");
		} else {
			for (int i = 0; i < AppState.alarms_count; i++) {
				len += sprintf(tx_buffer + len, "[%d] %02d:%02d - %s\r\n", i,
						AppState.alarms[i].hours, AppState.alarms[i].mins,
						AppState.alarms[i].active ? "ON" : "OFF");
			}
		}

		Reply(tx_buffer);
		return;
	}

	if (!isdigit((unsigned char )args[0])) {
		Reply("ERR: First char must be ID\r\n");
		return;
	}

	int id = args[0] - '0';
	char *time_str = args + 1;

	int8_t h, m;
	if (Parse_Time_Flexible(time_str, &h, &m)) {
		if (h >= 0 && h < 24 && m >= 0 && m < 60) {
			if (id < AppState.alarms_count) {
				AppState_EditAlarm(id, h, m);
				char buf[40];
				sprintf(buf, "OK: Alarm %d Modified to %02d:%02d\r\n", id, h,
						m);
				Reply(buf);
			} else {
				AppState_AddAlarm(h, m);
				char buf[50];
				int new_id = AppState.alarms_count - 1;
				sprintf(buf, "OK: New Alarm [%d] Added %02d:%02d\r\n", new_id,
						h, m);
				Reply(buf);
			}
		} else {
			Reply("ERR: Invalid Time\r\n");
		}
	} else {
		Reply("ERR: Format error. Try: a10830\r\n");
	}
}

static void Cmd_DeleteAlarm(char *args) {
	int id;
	if (sscanf(args, "%d", &id) == 1) {
		if (id < AppState.alarms_count) {
			AppState_DeleteAlarm((uint8_t) id);
			char buf[32];
			sprintf(buf, "OK: Alarm %d deleted\r\n", id);
			Reply(buf);
		} else {
			Reply("ERR: Alarm ID not found\r\n");
		}
	} else {
		Reply("ERR: ID required\r\n");
	}
}

static void Cmd_SetTime(char *args) {
	int h, m, s;
	bool parsed = false;

	if (sscanf(args, "%d%*c%d%*c%d", &h, &m, &s) == 3) {
		parsed = true;
	}

	else if (sscanf(args, "%2d%2d%2d", &h, &m, &s) == 3) {
		parsed = true;
	}

	if (parsed) {
		if (h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60) {
			DS3231_GetTime((RTC_Time_t*) &AppState.now);

			AppState.now.hours = (uint8_t) h;
			AppState.now.minutes = (uint8_t) m;
			AppState.now.seconds = (uint8_t) s;

			DS3231_SetTime((RTC_Time_t*) &AppState.now);

			char buf[40];
			sprintf(buf, "OK: Time Set %02d:%02d:%02d\r\n", AppState.now.hours,
					AppState.now.minutes, AppState.now.seconds);
			Reply(buf);
		} else {
			Reply("ERR: Invalid Range (0-23, 0-59)\r\n");
		}
	} else {
		Reply("ERR: Format HH:MM:SS or HHMMSS\r\n");
	}
}

static void Cmd_SetDate(char *args) {
	int d, m, y;
	if (sscanf(args, "%d%*c%d%*c%d", &d, &m, &y) == 3) {
		DS3231_GetTime((RTC_Time_t*) &AppState.now);
		AppState.now.day = d;
		AppState.now.month = m;
		AppState.now.year = y;
		DS3231_SetTime((RTC_Time_t*) &AppState.now);
		Reply("OK: Date Set\r\n");
	} else {
		Reply("ERR: Format DD.MM.YY\r\n");
	}
}

static void Cmd_Info(void) {
	int len = 0;
	const char *dow_str = AppState_GetDayOfWeekStr(AppState.now.day_of_week);
	len += sprintf(tx_buffer + len, "--- LAMP INFO ---\r\n");
	len += sprintf(tx_buffer + len, "Time: %02d:%02d:%02d\r\n",
			AppState.now.hours, AppState.now.minutes, AppState.now.seconds);
	len += sprintf(tx_buffer + len, "Date: %s, %02d/%02d/20%02d\r\n", dow_str,
			AppState.now.day, AppState.now.month, AppState.now.year);
	len += sprintf(tx_buffer + len, "Light: %s (%d%%)\r\n",
			AppState.is_light_on ? "ON" : "OFF", AppState.brightness);

	len += sprintf(tx_buffer + len, "Alarms (%d):\r\n", AppState.alarms_count);
	for (int i = 0; i < AppState.alarms_count; i++) {
		len += sprintf(tx_buffer + len, "[%d] %02d:%02d - %s\r\n", i,
				AppState.alarms[i].hours, AppState.alarms[i].mins,
				AppState.alarms[i].active ? "ON" : "OFF");
	}
	int bat_pct = 0;
	if (AppState.battery_voltage > 3.0f) {
		bat_pct = (int) ((AppState.battery_voltage - 3.0f) * 100.0f / 1.2f);
	}
	if (bat_pct > 100)
		bat_pct = 100;
	if (bat_pct < 0)
		bat_pct = 0;

	len += sprintf(tx_buffer + len, "Battery: %d.%02dV (%d%%)\r\n",
			(int) AppState.battery_voltage,
			(int) ((AppState.battery_voltage - (int) AppState.battery_voltage)
					* 100), bat_pct);
	len += sprintf(tx_buffer + len, "--- COMMANDS ---\r\n");
	len += sprintf(tx_buffer + len, "1 / 2      : Light ON / OFF\r\n");
	len += sprintf(tx_buffer + len, "b <0-100>  : Brightness (e.g. b 50)\r\n");
	len += sprintf(tx_buffer + len, "t <HHMMSS> : Set Time (e.g. t153000)\r\n");
	len += sprintf(tx_buffer + len,
			"t <H:M:S>  : Set Time (e.g. t 9:30:0)\r\n");
	len += sprintf(tx_buffer + len,
			"d <D.M.Y>  : Set Date (e.g. d 1.1.26)\r\n");
	len += sprintf(tx_buffer + len, "a          : List Alarms\r\n");
	len += sprintf(tx_buffer + len, "a<ID><HHMM>: Set/Edit (e.g. a00730)\r\n");
	len += sprintf(tx_buffer + len,
			"o <ID>     : Toggle ON/OFF (e.g. o 0)\r\n");
	len += sprintf(tx_buffer + len,
			"del <ID>   : Delete Alarm (e.g. del 0)\r\n");
	len += sprintf(tx_buffer + len, "r          : System Reset\r\n");

	if (len > 0) {
		Bluetooth_Send(tx_buffer);
		printf(">> SENT INFO (length: %d)\n", len);
	}
}

void CLI_ProcessCommand(char *input) {
	last_activity_time = HAL_GetTick();
	ssd1306_SetDisplayOn(1);
	while (*input == ' ')
		input++;
	if (*input == 0)
		return;
	size_t len = strlen(input);
	while (len > 0
			&& (input[len - 1] == '\r' || input[len - 1] == '\n'
					|| input[len - 1] == ' ')) {
		input[len - 1] = '\0';
		len--;
	}
	printf("\r\n[DEBUG] Input: '%s'\r\n", input);
	printf("[DEBUG] Hex: %02X %02X %02X ...\r\n", (uint8_t) input[0],
			(uint8_t) input[1], (uint8_t) input[2]);

	if (strncmp(input, "del", 3) == 0) {
		Cmd_DeleteAlarm(input + 3);
		return;
	}

	int cmd_len = 0;
	char cmd_char = Normalize_Command(input, &cmd_len);
	char *args = input + cmd_len;

	printf("[DEBUG] Norm Cmd: '%c', Args: '%s'\r\n", cmd_char, args);

	switch (cmd_char) {
	case '1':
		AppState.is_light_on = true;
		AppState.brightness = 100;
		Reply("OK: Light ON 100%\r\n");
		break;

	case '2':
		AppState.is_light_on = false;
		AppState.brightness = 0;
		Reply("OK: Light OFF\r\n");
		break;

	case 'b':
		Cmd_SetBrightness(args);
		break;

	case 'a':
		Cmd_SetAlarm(args);
		break;

	case 't':
		Cmd_SetTime(args);
		break;

	case 'd':
		Cmd_SetDate(args);
		break;

	case 'o': {
		int id;
		if (sscanf(args, "%d", &id) == 1) {
			if (id < AppState.alarms_count) {
				bool is_active = AppState_ToggleAlarm((uint8_t) id);
				int len = 0;
				len += sprintf(tx_buffer + len, "OK: Alarm [%d] turned %s\r\n",
						id, is_active ? "ON" : "OFF");

				if (is_active) {
					len += sprintf(tx_buffer + len, "Set at: %02d:%02d\r\n",
							AppState.alarms[id].hours,
							AppState.alarms[id].mins);

					char next_str[32];
					AppState_GetNextAlarmString(next_str);
					len += sprintf(tx_buffer + len, "%s\r\n", next_str);
				}

				Reply(tx_buffer);
			} else {
				Reply("ERR: Bad ID\r\n");
			}
		}
	}
		break;

	case 'i':
		Cmd_Info();
		break;

	case 'r':
		Reply("System Reset...\r\n");
		HAL_Delay(100);
		HAL_NVIC_SystemReset();
		break;

	default:
		Reply("Unknown Command.\r\n");
		break;
	}
}
