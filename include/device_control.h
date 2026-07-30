// SPDX-License-Identifier: CC-BY-SA-3.0

#pragma once

#include <stdint.h>

void CONTROL_handleUSB(void);
void CONTROL_emitInput(uint8_t control, uint8_t pressed);
void CONTROL_emitRotation(uint8_t control);
