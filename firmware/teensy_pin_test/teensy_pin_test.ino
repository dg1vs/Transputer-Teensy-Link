#include <Arduino.h>
#include "pin_config.h"

static int activeOutputPin = -1;
static bool outputState = false;
static uint32_t lastToggleUs = 0;
static uint32_t lastInputPrintMs = 0;

static char rxBuffer[32];
static uint8_t rxPos = 0;

// --------------------------------------------------
// Helpers
// --------------------------------------------------

bool isConfiguredOutputPin(uint8_t pin) {
  for (size_t i = 0; i < NUM_OUTPUT_PINS; ++i) {
    if (outputPins[i].pin == pin) {
      return true;
    }
  }
  return false;
}

void setupPins() {
  for (size_t i = 0; i < NUM_INPUT_PINS; ++i) {
    pinMode(inputPins[i].pin, INPUT_PULLUP);   // change to INPUT if needed
  }

  for (size_t i = 0; i < NUM_OUTPUT_PINS; ++i) {
    pinMode(outputPins[i].pin, OUTPUT);
    digitalWrite(outputPins[i].pin, LOW);
  }
}

void printHelp() {
  Serial.println();
  Serial.println("Teensy Pin Test ready");
  Serial.println("Enter only a pin number:");
  Serial.println("  2   -> output on pin 2");
  Serial.println("  23  -> output on pin 23");
  Serial.println("  0   -> stop output");
  Serial.println();
}

void printInputs() {
  Serial.print("Inputs: ");
  for (size_t i = 0; i < NUM_INPUT_PINS; ++i) {
    Serial.print(inputPins[i].pin);
    Serial.print("=");
    Serial.print(digitalRead(inputPins[i].pin) ? "H" : "L");
    if (i + 1 < NUM_INPUT_PINS) {
      Serial.print("  ");
    }
  }

  Serial.print("   |   Active OUT: ");
  if (activeOutputPin >= 0) {
    Serial.println(activeOutputPin);
  } else {
    Serial.println("none");
  }
}

void selectOutputPin(int pin) {
  // switch off old pin
  if (activeOutputPin >= 0) {
    digitalWrite((uint8_t)activeOutputPin, LOW);
  }

  outputState = false;
  lastToggleUs = micros();

  if (pin == 0) {
    activeOutputPin = -1;
    Serial.println("Output stopped");
    return;
  }

  if (pin < 0 || pin > 255 || !isConfiguredOutputPin((uint8_t)pin)) {
    activeOutputPin = -1;
    Serial.println("Error: pin is not a configured output");
    return;
  }

  activeOutputPin = pin;
  digitalWrite((uint8_t)activeOutputPin, LOW);

  Serial.print("Active output pin set to ");
  Serial.println(activeOutputPin);
}

void handleSerialLine(const char* line) {
  while (*line == ' ' || *line == '\t') {
    ++line;
  }

  if (*line == '\0') {
    return;
  }

  int pin = atoi(line);
  selectOutputPin(pin);
}

void readSerialInput() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r' || c == '\n') {
      if (rxPos > 0) {
        rxBuffer[rxPos] = '\0';
        handleSerialLine(rxBuffer);
        rxPos = 0;
      }
      continue;
    }

    if (rxPos < sizeof(rxBuffer) - 1) {
      rxBuffer[rxPos++] = c;
    } else {
      // overflow protection
      rxPos = 0;
      Serial.println("Error: input too long");
    }
  }
}

void updateOutputSignal() {
  if (activeOutputPin < 0) {
    return;
  }

  uint32_t now = micros();
  if ((uint32_t)(now - lastToggleUs) >= OUTPUT_TOGGLE_INTERVAL_US) {
    lastToggleUs = now;
    outputState = !outputState;
    digitalWrite((uint8_t)activeOutputPin, outputState ? HIGH : LOW);
  }
}

void updateInputDisplay() {
  uint32_t now = millis();
  if ((uint32_t)(now - lastInputPrintMs) >= INPUT_PRINT_INTERVAL_MS) {
    lastInputPrintMs = now;
    printInputs();
  }
}

// --------------------------------------------------
// Arduino
// --------------------------------------------------

void setup() {
  Serial.begin(115200);
  setupPins();

  delay(500);
  printHelp();
}

void loop() {
  readSerialInput();
  updateOutputSignal();
  updateInputDisplay();
}