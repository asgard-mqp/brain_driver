#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include "api.h"

void writeUart(uint8_t packet_id, int32_t value);
void readUart(void (*callback)(uint8_t packet_id, int32_t value));

#endif
