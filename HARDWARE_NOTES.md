# Tested hardware notes

These measurements apply only to the tested SinLoon SL2024502 sold under ASIN
`B0DN9T9J75`. Orientation is USB-C at the top.

| Function | CH552G GPIO | Package pin |
| --- | --- | ---: |
| K1, active low | P1.6 | 4 |
| K2, active low | P1.7 | 5 |
| K3, active low | P1.1 | 9 |
| K4, active low | P3.2 | 1 |
| K5, active low | P1.4 | 2 |
| K6, active low | P1.5 | 3 |
| Encoder phase A | P3.1 | 7 |
| Encoder phase B | P3.0 | 8 |
| Encoder push, active low | P3.3 | 10 |
| RGB chain data | P3.4 | 11 |
| SW2 upper pad | P1.7 | 5 |
| SW2 lower pad, through R10 1 kΩ | P3.2 | 1 |

The app's physical left-to-right keys map to internal controls
`2, 1, 0, 5, 4, 3`. The six LEDs use WS2812-compatible timing and GRB order.
Their chain order is K3, K2, K1, K6, K5, K4.

Confirmed USB identities are factory application `1189:8890` and WCH
bootloader `4348:55e0`. The Developer Preview application identity
`4249:4287` is experimental and is not an allocated public USB identity.
`1209:A6E1` has only been requested from pid.codes; it is not assigned and is
not active in the firmware or companion app.

## Confirmed SW2 recovery

1. Disconnect USB.
2. Bridge only the two SW2 pads while disconnected.
3. Connect through a reliable USB 2.0 path while holding the bridge.
4. Keep SW2 bridged for approximately 5–8 seconds.
5. Release after the terminal reports `found 4348:55e0`; a blue flash is not a
   reliable indicator.
6. Confirm `4348:55e0` before running an ISP operation.

Holding a key is not a reliable bootloader method for this board. Similar
looking boards can have different SW2 wiring.

## Flash safety boundary

Application flash ends before `0x3800` (14,336 bytes). The native flasher
erases only the number of 1 KiB code sectors needed by the validated image,
with a minimum of eight and maximum of fourteen sectors. It contains no
operation for DataFlash, configuration words, the ROM bootloader, OTP, or
protection removal.

Preflight validates CH552 chip ID `0x52`, sub-ID `0x11`, bootloader version
2.50, the observed stable configuration fields, and the application-size
boundary. A device UID is read only into memory because the WCH protocol uses
it to derive the volatile session key; it is neither displayed nor stored.

Flashing permanently replaces the manufacturer application. Its code could
not be read back, and no manufacturer firmware image or device-specific backup
is available. Factory restore is therefore not possible. Recovery exclusively
means returning to the protected WCH bootloader and flashing an Agent Micro
image again.

## Hardware acceptance record

The development board successfully exercised six key inputs, encoder
left/right/push, all six RGB pixels, keyboard press/release, Raw HID status,
physical events, live configuration, and emergency release-all. This is
evidence for the tested revision only, not a compatibility promise for other
boards.

## Porting to a different board

The table at the top is the full set of measurements this firmware depends on.
A board that differs in even one row needs those definitions changed and
rebuilt, and re-verified on that board.

A different number of keys or encoders cannot be handled by editing the table
alone. `ACTION_CONTROL_COUNT` (`include/input_actions.h`, currently 9) and
`LED_COUNT` (`include/led_control.h`, currently 6) are compile-time constants
that the Raw HID wire format, the companion app's control indices, and the LED
chain order all depend on. Changing them means revisiting `PROTOCOL.md` and the
app together.

This has not been done for any board other than the one measured here. Expect
to verify every row yourself, on hardware you accept losing.
