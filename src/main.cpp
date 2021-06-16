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
uint32_t loopStart;
uint32_t loopPeriod;

// Todo: Real spinning rainbow
// Todo: Make sure UpdateEffect() doesn't generate delay spikes
// Todo: Split rising edge/falling edge debouncing, delayed rising, straight falling
// Todo: Double byte messages
// Todo: Fix tic tac toe taking minutes to calculate a move

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    keypad.Init();
    rgb.Init();
    effectManager.AddEffect(new Rainbow());
    effectManager.AddEffect(new StaticRainbow());
    effectManager.AddEffect(new SpreadOut());
    effectManager.AddEffect(new StaticLight());
    effectManager.AddEffect(new Breathing());
    effectManager.AddEffect(new FractionalDrawingTest2d());
    effectManager.AddEffect(new SpinningRainbow());
    effectManager.AddEffect(new Ripple());
    effectManager.AddEffect(new AntiRipple());
    effectManager.AddEffect(new Stars());
    effectManager.AddEffect(new Raindrop());
    effectManager.AddEffect(new Snake());
    effectManager.AddEffect(new ShootingParticles());
    effectManager.AddEffect(new WhacAMole());
    effectManager.AddEffect(new TicTacToe());
    effectManager.AddEffect(new BullsNCows());

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
    rgb.Draw();
    CheckSerialMessage();

    // Don't change the measured loop time immediately as it might float around
    loopPeriod = (uint32_t)((loopPeriod * 0.8f) + ((micros() - loopStart) * 0.2f));
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
        static unsigned char incomingByte;
        static unsigned char incomingData;

        incomingByte = Serial.read();
        incomingData = incomingByte & 0b00111111;
        switch (incomingByte >> 6)
        {
            case 0: // UPDATERGBSTATE
                effectManager.SetEffect(incomingData);
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