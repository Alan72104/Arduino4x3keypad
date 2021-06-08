#include <Arduino.h>
#include "Keypad.h"

void Keypad::Init()
{
    // Columns are pulled up inputs
    // Leaving the pin normally HIGH
    // When connected to ground, the pin is pulled down to LOW
    for (uint8_t i : pinC)
    {
        pinMode(i, INPUT_PULLUP);
    }

    // Rows are voltage outputs
    // When writing to LOW, the pin will be grounded
    // When writing to HIGH, the pin will be connected to 5v
    for (uint8_t i : pinR)
    {
        pinMode(i, OUTPUT);
        digitalWrite(i, HIGH);
    }
}