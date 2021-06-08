#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>
#include "KeypadParams.h"

class Keypad
{
    private:
        const uint8_t pinC[WIDTH] = {8, 7, 6, 5};
        const uint8_t pinR[HEIGHT] = {2, 3, 4};
        const uint16_t scanPerSec = 2000u;
        const uint16_t microsPerScan = 1000000u / scanPerSec;
        uint8_t btnStateTemp;
        uint8_t btnState[HEIGHT][WIDTH];
        uint8_t lastBtnState[HEIGHT][WIDTH];
        const uint16_t debounceMicros = 1500u;
        uint32_t debounceTime[HEIGHT][WIDTH];
    public:
        void Init();
};

#endif