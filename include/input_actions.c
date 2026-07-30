// SPDX-License-Identifier: CC-BY-SA-3.0

#include "input_actions.h"

#include "delay.h"
#include "usb_conkbd.h"

#define ACTION_CMD_SET_BINDING 0x20
#define ACTION_MAX_STEPS 5

#define MOD_CTRL  0x01
#define MOD_SHIFT 0x02
#define MOD_ALT   0x04
#define MOD_GUI   0x08

typedef struct {
  uint8_t kind;
  uint8_t count;
  uint8_t modifier[ACTION_MAX_STEPS];
  uint8_t usage[ACTION_MAX_STEPS];
} ACTION_BINDING;

static __xdata ACTION_BINDING bindings[ACTION_CONTROL_COUNT];

static void setKeyboard(uint8_t control, uint8_t modifier, uint8_t usage) {
  uint8_t i;
  bindings[control].kind = ACTION_KEYBOARD;
  bindings[control].count = 1;
  bindings[control].modifier[0] = modifier;
  bindings[control].usage[0] = usage;
  for (i = 1; i < ACTION_MAX_STEPS; i++) {
    bindings[control].modifier[i] = 0;
    bindings[control].usage[i] = 0;
  }
}

void ACTION_init(void) {
  // Existing Codex profile: six direct shortcuts and F22/F23/F24 encoder
  // triggers consumed by CodexPad's reasoning automation.
  setKeyboard(0, MOD_GUI | MOD_SHIFT, 0x2F); // Previous Chat
  setKeyboard(1, MOD_GUI | MOD_ALT,   0x11); // Quick Chat
  setKeyboard(2, MOD_GUI | MOD_SHIFT, 0x30); // Next Chat
  setKeyboard(3, MOD_CTRL | MOD_SHIFT, 0x07); // Dictation
  setKeyboard(4, MOD_GUI, 0x11);              // New Chat
  setKeyboard(5, MOD_CTRL | MOD_SHIFT, 0x0A); // Review Tab
  setKeyboard(6, 0, 0x71); // F22, encoder left
  setKeyboard(7, 0, 0x72); // F23, encoder press
  setKeyboard(8, 0, 0x73); // F24, encoder right
}

void ACTION_trigger(uint8_t control) {
  ACTION_BINDING __xdata* binding;
  uint8_t i;
  uint16_t consumerUsage;
  if (control >= ACTION_CONTROL_COUNT) return;
  binding = &bindings[control];
  if (binding->kind == ACTION_KEYBOARD) {
    for (i = 0; i < binding->count && i < ACTION_MAX_STEPS; i++) {
      KBD_code_type(binding->modifier[i], binding->usage[i]);
      DLY_ms(2);
    }
  } else if (binding->kind == ACTION_CONSUMER && binding->count) {
    consumerUsage = ((uint16_t)binding->modifier[0] << 8) |
                    binding->usage[0];
    CON_type(consumerUsage);
  }
}

void ACTION_press(uint8_t control) {
  ACTION_BINDING __xdata* binding;
  if (control >= ACTION_CONTROL_COUNT) return;
  binding = &bindings[control];
  if (binding->kind == ACTION_HOLD_KEYBOARD && binding->count) {
    KBD_code_press(binding->modifier[0], binding->usage[0]);
    return;
  }
  if (binding->kind != ACTION_APP_ONLY) ACTION_trigger(control);
}

void ACTION_release(uint8_t control) {
  ACTION_BINDING __xdata* binding;
  if (control >= ACTION_CONTROL_COUNT) return;
  binding = &bindings[control];
  if (binding->kind == ACTION_HOLD_KEYBOARD && binding->count)
    KBD_code_release(binding->modifier[0], binding->usage[0]);
}

void ACTION_releaseAll(void) {
  KBD_releaseAll();
  CON_releaseAll();
}

void ACTION_applyPacket(const __xdata uint8_t* packet) {
  ACTION_BINDING __xdata* binding;
  uint8_t control;
  uint8_t i;
  if (packet[3] != ACTION_CMD_SET_BINDING) return;
  control = packet[4];
  if (control >= ACTION_CONTROL_COUNT || packet[5] > ACTION_HOLD_KEYBOARD ||
      packet[6] > ACTION_MAX_STEPS) return;
  ACTION_releaseAll();
  binding = &bindings[control];
  binding->kind = packet[5];
  binding->count = packet[6];
  for (i = 0; i < ACTION_MAX_STEPS; i++) {
    binding->modifier[i] = packet[7 + (i << 1)];
    binding->usage[i] = packet[8 + (i << 1)];
  }
}
