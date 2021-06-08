#include <Arduino.h>
#include "main.h"
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

    for (uint8_t j = 0; j < HEIGHT; j++)
        for (uint8_t i = 0; i < WIDTH; i++)
            btnState[j][i] = LOW;
}

void Keypad::ScanKeys()
{
    if (micros() - lastScanTime < (microsPerScan - (scanPeriod > microsPerScan) ? microsPerScan : scanPeriod)) return;
    lastScanTime = micros();

    for (uint8_t i = 0; i < HEIGHT; i++)
    {
        // Write the current row to LOW which is ground
        // If a switch on the row is pressed, it pulls the corresponding column input pin to ground which is LOW
        // According to the input_pullup characteristics, pin reading needs to be inverted
        digitalWrite(pinR[i], LOW);

        for (uint8_t j = 0; j < WIDTH; j++)
        {
            // Reading is inverted here
            btnStateTemp = !digitalRead(pinC[j]);

            if (btnStateTemp != lastBtnState[i][j])
            {
                if (debounceTime[i][j] == 0)
                    debounceTime[i][j] = micros();
            }

            if ((micros() - debounceTime[i][j]) > debounceMicros && debounceTime[i][j] != 0)
            {
                if (btnStateTemp != btnState[i][j])
                {
                    debounceTime[i][j] = 0;

                    btnState[i][j] = btnStateTemp;
#ifndef Debug
                    if (Serial.availableForWrite())
                    {
                        // Key status byte - |first 4 bits for key number, 3 zero padding bits, last one bit for pressed state|
                        Serial.write(((4 * i + j + 1) << 4) + (btnStateTemp == HIGH ? 1 : 0));
                    }
#endif
                    // This looks much simpler than // if (!(btnState[2][0] == HIGH && HandleModifier())) EffectHandleKey(btnStateTemp, j, i); //
                    // if (btnState[2][0] == HIGH && HandleModifier())
                    //     ;
                    // else
                    //     EffectHandleKey(btnStateTemp, j, i);
                }
            }
            lastBtnState[i][j] = btnStateTemp;
        }

        digitalWrite(pinR[i], HIGH);
    }

#ifdef Debug
    // Update frequency test
    static int t;
    static uint32_t tt;
    t++;
    if (millis() - tt >= 1000)
    {
        tt = millis();
        Serial.print(F("Updates per second: "));
        Serial.println(t);
        t = 0;
        Serial.print(F("Scan took: "));
        Serial.print(scanPeriod);
        Serial.println(F(" micros"));
    }
#endif

    scanPeriod = micros() - lastScanTime;
}