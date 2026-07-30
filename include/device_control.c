// SPDX-License-Identifier: CC-BY-SA-3.0

#include "device_control.h"

#include <stdint.h>

#include "input_actions.h"
#include "led_control.h"
#include "usb_hid.h"
#include "usb_identity.h"

#define CONTROL_PACKET_SIZE 32
#define CONTROL_PROTOCOL_VERSION 2
#define CONTROL_CMD_STATUS 0x30
#define CONTROL_CMD_RELEASE_ALL 0x31
#define CONTROL_REPORT_STATUS 0x80
#define CONTROL_REPORT_INPUT 0x81

static uint8_t sequence = 0;
static uint16_t pressedMask = 0;
static __code uint8_t visibleControl[9] = {2, 1, 0, 5, 4, 3, 6, 7, 8};

static void sendPacket(__xdata uint8_t* packet) {
  uint8_t i;
  uint8_t checksum = 0;
  for (i = 0; i < CONTROL_PACKET_SIZE - 1; i++) checksum ^= packet[i];
  packet[CONTROL_PACKET_SIZE - 1] = checksum;
  HID_sendRawReport(packet, CONTROL_PACKET_SIZE);
}

static void sendStatus(void) {
  __xdata uint8_t packet[CONTROL_PACKET_SIZE];
  uint8_t i;
  for (i = 0; i < CONTROL_PACKET_SIZE; i++) packet[i] = 0;
  packet[0] = 'C'; packet[1] = 'P'; packet[2] = CONTROL_PROTOCOL_VERSION;
  packet[3] = CONTROL_REPORT_STATUS;
  packet[4] = AGENT_MICRO_VERSION_MAJOR;
  packet[5] = AGENT_MICRO_VERSION_MINOR;
  packet[6] = AGENT_MICRO_VERSION_PATCH;
  packet[7] = 0x1F; // edges, events, hold, status, release-all
  packet[8] = 9; packet[9] = 6;
  packet[10] = (uint8_t)pressedMask;
  packet[11] = (uint8_t)(pressedMask >> 8);
  sendPacket(packet);
}

void CONTROL_emitInput(uint8_t control, uint8_t pressed) {
  __xdata uint8_t packet[CONTROL_PACKET_SIZE];
  uint8_t i;
  uint8_t visible;
  if (control >= 9) return;
  visible = visibleControl[control];
  if (pressed) pressedMask |= ((uint16_t)1 << visible);
  else pressedMask &= ~((uint16_t)1 << visible);
  for (i = 0; i < CONTROL_PACKET_SIZE; i++) packet[i] = 0;
  packet[0] = 'C'; packet[1] = 'P'; packet[2] = CONTROL_PROTOCOL_VERSION;
  packet[3] = CONTROL_REPORT_INPUT; packet[4] = ++sequence;
  packet[5] = visible; packet[6] = pressed ? 1 : 0;
  packet[7] = (uint8_t)pressedMask; packet[8] = (uint8_t)(pressedMask >> 8);
  sendPacket(packet);
}

void CONTROL_emitRotation(uint8_t control) {
  __xdata uint8_t packet[CONTROL_PACKET_SIZE];
  uint8_t i;
  if (control >= 9) return;
  for (i = 0; i < CONTROL_PACKET_SIZE; i++) packet[i] = 0;
  packet[0] = 'C'; packet[1] = 'P'; packet[2] = CONTROL_PROTOCOL_VERSION;
  packet[3] = CONTROL_REPORT_INPUT; packet[4] = ++sequence;
  packet[5] = visibleControl[control]; packet[6] = 2;
  packet[7] = (uint8_t)pressedMask; packet[8] = (uint8_t)(pressedMask >> 8);
  sendPacket(packet);
}

void CONTROL_handleUSB(void) {
  __xdata uint8_t packet[CONTROL_PACKET_SIZE];
  uint8_t checksum = 0;
  uint8_t count;
  uint8_t i;

  count = HID_available();
  if (!count) return;
  if (count != CONTROL_PACKET_SIZE) {
    while (HID_available()) (void)HID_read();
    HID_ack();
    return;
  }
  for (i = 0; i < CONTROL_PACKET_SIZE; i++)
    packet[i] = (uint8_t)HID_read();
  HID_ack();

  for (i = 0; i < CONTROL_PACKET_SIZE - 1; i++) checksum ^= packet[i];
  if (packet[0] != 'C' || packet[1] != 'P' ||
      (packet[2] != 1 && packet[2] != CONTROL_PROTOCOL_VERSION) ||
      checksum != packet[CONTROL_PACKET_SIZE - 1]) return;

  if (packet[3] >= 0x10 && packet[3] <= 0x12)
    LED_applyPacket(packet);
  else if (packet[3] == 0x20)
    ACTION_applyPacket(packet);
  else if (packet[3] == CONTROL_CMD_STATUS)
    sendStatus();
  else if (packet[3] == CONTROL_CMD_RELEASE_ALL) {
    ACTION_releaseAll();
    pressedMask = 0;
    sendStatus();
  }
}
