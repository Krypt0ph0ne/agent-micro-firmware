// SPDX-License-Identifier: CC-BY-SA-3.0

// Agent Micro firmware for the confirmed six-key CH552G hardware.
//
// Inputs:
//   K1..K6: P1.6, P1.7, P1.1, P3.2, P1.4, P1.5
//   Encoder A/B/push: P3.1, P3.0, P3.3
// Output:
//   Six WS2812-compatible GRB LEDs on P3.4
//
// SW2 remains the confirmed physical bootloader/recovery method.

#include <config.h>
#include <delay.h>
#include <device_control.h>
#include <gpio.h>
#include <input_actions.h>
#include <led_control.h>
#include <system.h>
#include <usb_conkbd.h>
#include <usb_descr.h>

#define HANDLE_INPUT(pin, now, last, control)    \
  do {                                           \
    (now) = PIN_read(pin);                       \
    if ((last) && !(now)) {                      \
      DLY_ms(5);                                 \
      if (!PIN_read(pin)) {                      \
        CONTROL_emitInput((control), 1);         \
        ACTION_press((control));                 \
      }                                          \
    } else if (!(last) && (now)) {               \
      DLY_ms(5);                                 \
      if (PIN_read(pin)) {                       \
        ACTION_release((control));               \
        CONTROL_emitInput((control), 0);         \
      }                                          \
    }                                            \
    (last) = PIN_read(pin);                      \
  } while (0)

void USB_interrupt(void);
void USB_ISR(void) __interrupt(INT_NO_USB) { USB_interrupt(); }

void main(void) {
  uint8_t key1Now, key1Last;
  uint8_t key2Now, key2Last;
  uint8_t key3Now, key3Last;
  uint8_t key4Now, key4Last;
  uint8_t key5Now, key5Last;
  uint8_t key6Now, key6Last;
  uint8_t encoderANow, encoderALast;
  uint8_t encoderSwitchNow, encoderSwitchLast;
  uint8_t tickDivider = 0;

  CLK_config();
  DLY_ms(5);

  PIN_input_PU(PIN_KEY_1);
  PIN_input_PU(PIN_KEY_2);
  PIN_input_PU(PIN_KEY_3);
  PIN_input_PU(PIN_KEY_4);
  PIN_input_PU(PIN_KEY_5);
  PIN_input_PU(PIN_KEY_6);
  PIN_input_PU(PIN_ENC_A);
  PIN_input_PU(PIN_ENC_B);
  PIN_input_PU(PIN_ENC_SW);
  DLY_ms(20);

  LED_init();
  KBD_init();
  ACTION_init();
  WDT_start();

  key1Last = PIN_read(PIN_KEY_1);
  key2Last = PIN_read(PIN_KEY_2);
  key3Last = PIN_read(PIN_KEY_3);
  key4Last = PIN_read(PIN_KEY_4);
  key5Last = PIN_read(PIN_KEY_5);
  key6Last = PIN_read(PIN_KEY_6);
  encoderALast = PIN_read(PIN_ENC_A);
  encoderSwitchLast = PIN_read(PIN_ENC_SW);

  while (1) {
    HANDLE_INPUT(PIN_KEY_1, key1Now, key1Last, 0);
    HANDLE_INPUT(PIN_KEY_2, key2Now, key2Last, 1);
    HANDLE_INPUT(PIN_KEY_3, key3Now, key3Last, 2);
    HANDLE_INPUT(PIN_KEY_4, key4Now, key4Last, 3);
    HANDLE_INPUT(PIN_KEY_5, key5Now, key5Last, 4);
    HANDLE_INPUT(PIN_KEY_6, key6Now, key6Last, 5);

    encoderANow = PIN_read(PIN_ENC_A);
    if (encoderALast && !encoderANow) {
      if (PIN_read(PIN_ENC_B)) {
        ACTION_trigger(8);
        CONTROL_emitRotation(8);
      } else {
        ACTION_trigger(6);
        CONTROL_emitRotation(6);
      }
      DLY_ms(3);
    }
    encoderALast = encoderANow;

    HANDLE_INPUT(PIN_ENC_SW, encoderSwitchNow, encoderSwitchLast, 7);

    CONTROL_handleUSB();
    if (++tickDivider >= 20) {
      tickDivider = 0;
      LED_tick20ms();
    }

    WDT_reset();
    DLY_ms(1);
  }
}
