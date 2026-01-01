#include "bluetooth.h"
#include "usart.h"

extern UART_HandleTypeDef huart2;

uint8_t bt_rx_buffer[BT_RX_BUFFER_SIZE];
char bt_command_buffer[BT_RX_BUFFER_SIZE];

volatile uint8_t bt_command_received = 0;

void Bluetooth_Init(void) {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, bt_rx_buffer, BT_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);
}

void Bluetooth_Send(char* message) {
    uint16_t len = strlen(message);
    HAL_UART_Transmit_DMA(&huart2, (uint8_t*)message, len);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART2) {
        memcpy(bt_command_buffer, bt_rx_buffer, Size);
        bt_command_buffer[Size] = '\0';
        bt_command_received = 1;

        HAL_UARTEx_ReceiveToIdle_DMA(huart, bt_rx_buffer, BT_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
    }
}
