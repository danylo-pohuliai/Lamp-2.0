#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

#define BT_RX_BUFFER_SIZE 64

extern volatile uint8_t bt_command_received;
extern char bt_command_buffer[BT_RX_BUFFER_SIZE];

void Bluetooth_Init(void);
void Bluetooth_Send(char* message);

#endif
