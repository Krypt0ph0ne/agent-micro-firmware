// SPDX-License-Identifier: CC-BY-SA-3.0

#pragma once

/*
 * The legacy 4249:4287 identity is retained only for experimental source
 * builds. It is not an allocated public USB identity. A stable release must
 * not be published until pid.codes has assigned an identity under VID 0x1209.
 */
#define AGENT_MICRO_USB_VENDOR_ID       0x4249
#define AGENT_MICRO_USB_PRODUCT_ID      0x4287

#define AGENT_MICRO_VERSION_MAJOR       1
#define AGENT_MICRO_VERSION_MINOR       0
#define AGENT_MICRO_VERSION_PATCH       0
#define AGENT_MICRO_USB_DEVICE_VERSION  0x0100
