// SPDX-License-Identifier: CC-BY-SA-3.0

// ===================================================================================
// User Configurations for CH552E USB MacroPad Mini
// ===================================================================================

#pragma once

#include "usb_identity.h"

// Firmware-confirmed pin definitions
#define PIN_NEO             P34         // physical pin 11, RGB chain data
#define PIN_KEY_1           P16         // physical pin 4
#define PIN_KEY_2           P17         // physical pin 5
#define PIN_KEY_3           P11         // physical pin 9
#define PIN_KEY_4           P32         // physical pin 1
#define PIN_KEY_5           P14         // physical pin 2
#define PIN_KEY_6           P15         // physical pin 3
#define PIN_ENC_A           P31         // physical pin 7
#define PIN_ENC_B           P30         // physical pin 8
#define PIN_ENC_SW          P33         // physical pin 10

// NeoPixel configuration
#define NEO_GRB                         // type of pixel: NEO_GRB or NEO_RGB
#define NEO_MIN             0
#define NEO_GLOW            0.4
#define NEO_MAX             1.0

// USB device descriptor
#define USB_VENDOR_ID       AGENT_MICRO_USB_VENDOR_ID
#define USB_PRODUCT_ID      AGENT_MICRO_USB_PRODUCT_ID
#define USB_DEVICE_VERSION  AGENT_MICRO_USB_DEVICE_VERSION

// USB configuration descriptor
#define USB_MAX_POWER_mA    50          // max power in mA

// USB descriptor strings
#define MANUFACTURER_STR    'A','g','e','n','t',' ','M','i','c','r','o'
#define PRODUCT_STR         'A','g','e','n','t',' ','M','i','c','r','o',' ','C','H','5','5','2'
#define INTERFACE_STR       'H','I','D','-','K','e','y','b','o','a','r','d'
