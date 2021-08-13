#include <Arduino.h>
#include <FastLED.h>
#include "Effect.h"
#include "EffectManager.h"
#include "Keypad.h"
#include "KeypadParams.h"
#include "main.h"
#include "Rgb.h"

Keypad keypad;
Rgb rgb;
EffectManager effectManager;
uint32_t loopStart;
uint32_t loopPeriod;

// Todo: Split rising edge/falling edge debouncing, delayed rising, straight falling
// Todo: Fix tic tac toe taking minutes to calculate a move

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    keypad.Init();
    rgb.Init();
    effectManager.Init();

    // Wait until the serial system starts
    while (!Serial) {}
    Serial.begin(19200);
}

void loop()
{
    loopStart = micros();
    
    UpdateLed();
    keypad.ScanKeys();
    effectManager.UpdateEffect();
    rgb.Show();
    CheckSerialMessage();

    // Only update the measured loop time by 80% new measurement as it might float around
    loopPeriod = (uint32_t)((loopPeriod * 0.2f) + ((micros() - loopStart) * 0.8f));
#ifdef Debug
    static uint32_t t = 0ul;
    if (millis() - t >= 1000)
    {
        t = millis();
        Serial.print(F("Main loop took: "));
        Serial.print(loopPeriod);
        Serial.println(F(" micros"));
    }
#endif
}

uint32_t GetLoopTime() { return loopPeriod; }

void CheckSerialMessage()
{
    // If serial has received bytes, read the driver messages
    if (Serial.available())
    {
        static uint8_t incomingByte[2];
        static uint8_t imcomingByteIndex;
        static uint8_t data;

        incomingByte[imcomingByteIndex++] = Serial.read();
        if (imcomingByteIndex >= 2)
        {
            imcomingByteIndex = 0;
            data = incomingByte[1];
            switch (incomingByte[0])
            {
                case 0: // UPDATERGBSTATE
                    effectManager.SetEffect(data);
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
                case 4: // SETRGBBRIGHTNESS
                    rgb.SetBrightness(data);
                    break;
                case 5: // INCREASEEFFECTSPEED
                    effectManager.IncreaseEffectSpeed();
                    break;
                case 6: // DECREASEEFFECTSPEED
                    effectManager.DecreaseEffectSpeed();
                    break;
                case 7: // ENABLEMODIFIEDKEYS
                    keypad.EnableModifiedKeys();
                    break;
                case 8: // DISABLEMODIFIEDKEYS
                    keypad.DisableModifiedKeys();
                    break;
                default: break;
            }
        }
    }
}

// Random without repeat
uint16_t Random(uint16_t max)
{
    static uint16_t last = 0l;
    if (max == 0)
        return 0;
    else
    {
        uint16_t rtn = random(max);
        while (rtn == last)
            rtn = random(max);
        last = rtn;
        return rtn;
    }
}

// Random float
float frandom(float min, float max)
{
    return min + random(1ul << 31) * (max - min) / (1ul << 31);
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