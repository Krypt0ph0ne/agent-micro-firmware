// SPDX-License-Identifier: CC-BY-SA-3.0

#pragma once

/*
 * Developer Preview source builds use the experimental 4249:4287 identity.
 * It is not an allocated public USB identity. 1209:A6E1 is requested only;
 * it is not assigned or active. Do not publish a stable binary unless and
 * until an appropriate identity is assigned and applied.
 */
#define AGENT_MICRO_USB_VENDOR_ID       0x4249
#define AGENT_MICRO_USB_PRODUCT_ID      0x4287

#define AGENT_MICRO_VERSION_MAJOR       1
#define AGENT_MICRO_VERSION_MINOR       0
#define AGENT_MICRO_VERSION_PATCH       0
#define AGENT_MICRO_USB_DEVICE_VERSION  0x0100
