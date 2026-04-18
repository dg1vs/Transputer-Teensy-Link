#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

// -----------------------------
// Konfiguration
// -----------------------------

struct PinEntry {
  uint8_t pin;
  const char* name;
};

// Relevante Eingänge
static const PinEntry inputPins[] = {
  {28,  "IN_28"},
  {32,  "IN_32"},
};

// Relevante Ausgänge
static const PinEntry outputPins[] = {
  {29,  "OUT_29"},
  {30,  "OUT_30"},
  {31,  "OUT_31"}
};

static const size_t NUM_INPUT_PINS  = sizeof(inputPins) / sizeof(inputPins[0]);
static const size_t NUM_OUTPUT_PINS = sizeof(outputPins) / sizeof(outputPins[0]);

// Input display interval
static const uint32_t INPUT_PRINT_INTERVAL_MS = 100;

// 5 kHz square wave => period 200 us => toggle every 100 us
static const uint32_t OUTPUT_TOGGLE_INTERVAL_US = 100;

#endif