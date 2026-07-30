// SPDX-License-Identifier: CC-BY-SA-3.0

#include "led_control.h"

#include "neo.h"
#define LED_CMD_SET_ONE 0x10
#define LED_CMD_SET_ALL 0x11
#define LED_CMD_ALL_OFF 0x12

typedef struct {
  uint8_t effect;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t period20ms;
  uint8_t brightness;
} LED_CONFIG;

static __xdata LED_CONFIG ledConfig[LED_COUNT];
static __xdata uint8_t frameRed[LED_COUNT];
static __xdata uint8_t frameGreen[LED_COUNT];
static __xdata uint8_t frameBlue[LED_COUNT];
static uint16_t animationTick;

static uint8_t scaleChannel(uint8_t channel, uint8_t level) {
  return (uint8_t)(((uint16_t)channel * level) / 255);
}

static uint8_t effectLevel(const LED_CONFIG* config) {
  uint8_t period = config->period20ms < 5 ? 5 : config->period20ms;
  uint8_t phase = (uint8_t)(animationTick % period);
  uint8_t half;

  switch (config->effect) {
    case LED_EFFECT_STEADY:
      return config->brightness;
    case LED_EFFECT_BLINK:
      return phase < ((period + 1) >> 1) ? config->brightness : 0;
    case LED_EFFECT_PULSE:
      half = period >> 1;
      if (!half) return config->brightness;
      if (phase <= half)
        return (uint8_t)(((uint16_t)config->brightness * phase) / half);
      return (uint8_t)(((uint16_t)config->brightness * (period - phase)) /
                       (period - half));
    default:
      return 0;
  }
}

static void prepareLogicalLED(uint8_t logicalIndex) {
  LED_CONFIG* config = &ledConfig[logicalIndex];
  uint8_t level = effectLevel(config);
  frameRed[logicalIndex] = scaleChannel(config->red, level);
  frameGreen[logicalIndex] = scaleChannel(config->green, level);
  frameBlue[logicalIndex] = scaleChannel(config->blue, level);
}

static void render(void) {
  uint8_t i;

  // All division/modulo and XRAM configuration work must finish before the
  // first pixel bit. A long calculation between pixels is interpreted as a
  // latch by this LED chain, which would repeatedly update only pixel zero.
  for (i = 0; i < LED_COUNT; i++) prepareLogicalLED(i);

  NEO_latch();
  EA = 0;
  // Confirmed physical chain: K3, K2, K1, K6, K5, K4.
  NEO_writeColor(frameRed[2], frameGreen[2], frameBlue[2]);
  NEO_writeColor(frameRed[1], frameGreen[1], frameBlue[1]);
  NEO_writeColor(frameRed[0], frameGreen[0], frameBlue[0]);
  NEO_writeColor(frameRed[5], frameGreen[5], frameBlue[5]);
  NEO_writeColor(frameRed[4], frameGreen[4], frameBlue[4]);
  NEO_writeColor(frameRed[3], frameGreen[3], frameBlue[3]);
  EA = 1;
  NEO_latch();
}

static void setFromPacket(uint8_t index, const uint8_t* packet) {
  LED_CONFIG* config;
  if (index >= LED_COUNT) return;
  config = &ledConfig[index];
  config->effect = packet[5] <= LED_EFFECT_PULSE ? packet[5] : LED_EFFECT_OFF;
  config->red = packet[6];
  config->green = packet[7];
  config->blue = packet[8];
  config->period20ms = packet[9] < 5 ? 5 : packet[9];
  config->brightness = packet[10];
}

void LED_init(void) {
  uint8_t i;
  NEO_init();
  animationTick = 0;
  for (i = 0; i < LED_COUNT; i++) {
    ledConfig[i].effect = LED_EFFECT_STEADY;
    ledConfig[i].red = 255;
    ledConfig[i].green = 255;
    ledConfig[i].blue = 255;
    ledConfig[i].period20ms = 50;
    ledConfig[i].brightness = 48;
  }
  render();
}

void LED_applyPacket(const __xdata uint8_t* packet) {
  uint8_t i;

  switch (packet[3]) {
    case LED_CMD_SET_ONE:
      setFromPacket(packet[4], packet);
      break;
    case LED_CMD_SET_ALL:
      for (i = 0; i < LED_COUNT; i++) setFromPacket(i, packet);
      break;
    case LED_CMD_ALL_OFF:
      for (i = 0; i < LED_COUNT; i++) ledConfig[i].effect = LED_EFFECT_OFF;
      break;
    default:
      return;
  }
  render();
}

void LED_tick20ms(void) {
  animationTick++;
  render();
}
