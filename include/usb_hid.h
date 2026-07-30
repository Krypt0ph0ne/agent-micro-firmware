// SPDX-License-Identifier: CC-BY-SA-3.0

// ===================================================================================
// USB HID Functions for CH551, CH552 and CH554
// ===================================================================================

#pragma once
#include <stdint.h>

void HID_init(void);                                    // setup USB-HID
void HID_sendReport(__xdata uint8_t *buf, uint8_t len); // send HID report
void HID_sendRawReport(__xdata uint8_t *buf, uint8_t len);
uint8_t HID_statusLed(void);
uint8_t HID_available(void);
void HID_ack(void);
char HID_read(void);
