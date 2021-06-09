#include <Arduino.h>
#include <FastLED.h>
#include "main.h"
#include "KeypadParams.h"
#include "Keypad.h"
#include "Rgb.h"
#include "Effect.h"
#include "EffectManager.h"
#include "Effects/Effects.h"

Keypad keypad;
Rgb rgb;
EffectManager effectManager;

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
    rgb.Init();
    effectManager.AddEffect(new StaticRainbow());
    effectManager.AddEffect(new Rainbow());
    effectManager.AddEffect(new SpreadOut());
    effectManager.AddEffect(new StaticLight());
    effectManager.AddEffect(new Breathing());
    effectManager.AddEffect(new FractionalDrawingTest2d());

    // Wait until the serial system starts
    while (!Serial) {}
    Serial.begin(19200);
}

void loop()
{
    UpdateLed();
    keypad.ScanKeys();
    effectManager.UpdateEffect();
    rgb.Draw();
    CheckSerialMessage();
}

void CheckSerialMessage()
{
    // If serial has received bytes, read the driver messages
    if (Serial.available())
    {
        static unsigned char incomingByte;
        static unsigned char incomingData;

        incomingByte = Serial.read();
        incomingData = incomingByte & 0b00111111;
        switch (incomingByte >> 6)
        {
            case 0: // UPDATERGBSTATE
                // rgbState = (RgbState)incomingData;
                break;
            case 1: // GETRGBDATA
                static CHSV rgbToHsv;
                static CRGB brightenRgb;
                for (int i = 0; i < WIDTH * HEIGHT; i++)
                {
                    rgbToHsv = rgb2hsv_approximate(rgb.GetColor(i));
                    brightenRgb.setHSV(rgbToHsv[0], rgbToHsv[1], (255 - 200) * (rgbToHsv[2] - 0) / (255 - 0) + 200);
                    Serial.write(brightenRgb[0]);
                    Serial.write(brightenRgb[1]);
                    Serial.write(brightenRgb[2]);
                }
                break;
            case 2: // INCREASERGBBRIGHTNESS
                rgb.IncreaseBrightness();
                break;
            case 3: // DECREASERGBBRIGHTNESS
                rgb.DecreaseBrightness();
                break;
        }
    }
}

void UpdateLed()
{
    static uint8_t ledState = LOW;
    static uint32_t lastBlinkTime = 0ul;
    static const uint32_t ledBlinkLen = 5ul;

    if (ledState == LOW)
    {
        if (millis() - lastBlinkTime > 1000ul - ledBlinkLen)
        {
            lastBlinkTime = millis();
            ledState = HIGH;
            digitalWrite(LED_BUILTIN, ledState);
        }
    }
    else
    {
        if (millis() - lastBlinkTime > ledBlinkLen)
        {
            lastBlinkTime = millis();
            ledState = LOW;
            digitalWrite(LED_BUILTIN, ledState);
        }
    }
}