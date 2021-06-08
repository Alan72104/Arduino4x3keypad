#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "Keypad.h"
#include "Led.h"

Keypad keypad;

// Todo: Real spinning rainbow
// Todo: Make sure UpdateEffect() doesn't generate delay spikes
// Todo: Split rising edge/falling edge debouncing, delayed rising, straight falling
// Todo: Split codes
// Todo: Double byte messages
// Todo: Fill up the last 40% of the flash with more effects!
// Todo: Why tf does ScanKeys() run 3k times per sec???
// Todo: Fix tic tac toe taking minutes to calculate a move

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    keypad.Init();


    // Wait until the serial system starts
    while (!Serial) {}
    Serial.begin(19200);
}

void loop()
{

}