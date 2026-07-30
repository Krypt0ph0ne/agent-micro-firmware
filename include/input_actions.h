// SPDX-License-Identifier: CC-BY-SA-3.0

#pragma once

#include <stdint.h>

#define ACTION_CONTROL_COUNT 9

enum ACTION_KIND {
  ACTION_DISABLED = 0,
  ACTION_KEYBOARD = 1,
  ACTION_CONSUMER = 2,
  ACTION_APP_ONLY = 3,
  ACTION_HOLD_KEYBOARD = 4
};

void ACTION_init(void);
void ACTION_trigger(uint8_t control);
void ACTION_press(uint8_t control);
void ACTION_release(uint8_t control);
void ACTION_releaseAll(void);
void ACTION_applyPacket(const __xdata uint8_t* packet);
