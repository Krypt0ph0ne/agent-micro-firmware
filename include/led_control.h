// SPDX-License-Identifier: CC-BY-SA-3.0

#pragma once

#include <stdint.h>

#define LED_COUNT 6

enum LED_EFFECT {
  LED_EFFECT_OFF = 0,
  LED_EFFECT_STEADY = 1,
  LED_EFFECT_BLINK = 2,
  LED_EFFECT_PULSE = 3
};

void LED_init(void);
void LED_applyPacket(const __xdata uint8_t* packet);
void LED_tick20ms(void);
