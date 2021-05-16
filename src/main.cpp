#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "main.h"

// Variables required to run the keypad
const uint8_t pinC[WIDTH] = {8, 7, 6, 5};
const uint8_t pinR[HEIGHT] = {2, 3, 4};
uint8_t btnStateTemp = LOW;
uint8_t btnState[HEIGHT][WIDTH];
uint8_t ledState = LOW;
unsigned long lastBlinkTime = 0ul;
unsigned long ledBlinkLen = 5ul;
unsigned long loopEndTime = 0ul;
unsigned long loopStartTime = 0ul;
unsigned long loopPeriod = 0ul;
const uint16_t scanPerSec = 1000u;
const uint16_t microsPerScan = 1000000u / scanPerSec;

// Variables for the rgb leds
CRGB leds[NUM_LEDS];
uint8_t rgbBrightness = 63;
unsigned long lastRgbBrightnessChange = 0ul;

// Variables for the rgb effects
RgbState rgbState = staticLight;
RgbState lastRgbState = staticLight;
unsigned long lastRgbStateChange = 0ul;
std::vector<Ball> balls;
float fractionalDrawingTestY = 0.0f;
float fractionalDrawingTestX = 0.0f;
std::vector<Circle> circles;
std::vector<Raindrop> raindrops;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

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
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  // Wait until the serial system starts
  while (!Serial) {}
  Serial.begin(19200);
}

// #define Debug

void loop() {
  // The start time of the FULL loop
  loopStartTime = micros();

  UpdateLed();
  UpdateEffect();
  UpdateRgb();

  // Todo: Split key detecting into its own function
  if (micros() - loopEndTime < (microsPerScan - (loopPeriod > microsPerScan ? microsPerScan : loopPeriod))) return;
  
  for (uint8_t i = 0; i < HEIGHT; i++)
  {
    // Write the current row to LOW which is ground
    // If a switch on the row is pressed, it pulls the corresponding column input pin to ground which is LOW
    // According to the input_pullup characteristics, pin reading needs to be inverted
    digitalWrite(pinR[i], LOW);

    for (uint8_t j = 0; j < WIDTH; j++)
    {
      btnStateTemp = digitalRead(pinC[j]);

      // Only perform actions when the state changes
      if (btnState[i][j] != btnStateTemp)
      {
        btnState[i][j] = btnStateTemp;

#ifndef Debug
        if (Serial.availableForWrite())
        {
          // Key status byte - |first 4 bits for key number, 3 zero padding bits, last one bit for pressed state|
          Serial.write(((4*i+j+1) << 4 ) + (btnStateTemp == LOW ? 1 : 0));
        }
#endif

        // Modifier key handling
        if (btnState[2][0] == !HIGH)
        {
          if (btnState[0][3] == !HIGH && millis() - lastRgbStateChange >= 150)
          {
            lastRgbStateChange = millis();
            NextRgbState(); 
          }
          else if (btnState[0][0] == !HIGH && millis() - lastRgbBrightnessChange >= 100)
          {
            lastRgbBrightnessChange = millis();
            rgbBrightness = min(rgbBrightness + 10, 255);
          }
          else if (btnState[0][1] == !HIGH && millis() - lastRgbBrightnessChange >= 100)
          {
            lastRgbBrightnessChange = millis();
            rgbBrightness = max(0, rgbBrightness - 10);
          }
        }
        
        // State specific handling
        switch (rgbState)
        {
          case spreadOut:
            if (btnStateTemp == !HIGH && balls.size() < 16)
            {
              CRGB color = CHSV(random(256), 255, rgbBrightness);
              balls.push_back(MakeBall(j, i, -1, color));
              balls.push_back(MakeBall(j, i, 1, color));
            }
            break;
          
          case fractionalDrawingTest2d:
            if (btnState[0][0] == !HIGH && fractionalDrawingTestY > 0.0f)         fractionalDrawingTestY -= 0.1f;
            else if (btnState[1][0] == !HIGH && fractionalDrawingTestY < HEIGHT)  fractionalDrawingTestY += 0.1f;
            else if (btnState[1][2] == !HIGH && fractionalDrawingTestX < WIDTH)   fractionalDrawingTestX += 0.1f;
            else if (btnState[1][1] == !HIGH && fractionalDrawingTestX > 0.0f)    fractionalDrawingTestX -= 0.1f;
            break;
          
          case ripple:
            if (btnStateTemp == !HIGH && circles.size() < 16)
              circles.push_back(MakeCircle(j, i, 0, CRGB(CHSV(random(256), 255, rgbBrightness))));
            break;
          
          case antiRipple:
            if (btnStateTemp == !HIGH && circles.size() < 16)
              circles.push_back(MakeCircle(j, i, 5.0f, CRGB(CHSV(random(256), 255, rgbBrightness))));
            break;
          
          default:
            break;
        }
      }
    }
    digitalWrite(pinR[i], HIGH);
  }

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
        rgbState = (RgbState)incomingData;
        break;
      case 1: // GETRGBDATA
        static CHSV rgbToHsv;
        static CRGB brightenRgb;
        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
          rgbToHsv = rgb2hsv_approximate(leds[i]);
          brightenRgb.setHSV(rgbToHsv[0], rgbToHsv[1], (255 - 200) * (rgbToHsv[2] - 0) / (255 - 0) + 200);
          Serial.write(brightenRgb[0]);
          Serial.write(brightenRgb[1]);
          Serial.write(brightenRgb[2]);
        }
        break;
      case 2: // INCREASERGBBRIGHTNESS
        rgbBrightness = min(rgbBrightness + 10, 255);
        break;
      case 3: // DECREASERGBBRIGHTNESS
        rgbBrightness = max(0, rgbBrightness - 10);
        break;
    }
  }

#ifdef Debug
  // Update frequency test
  static int t;
  static unsigned long tt;
  t++;
  if (millis() - tt >= 1000)
  {
    tt = millis();
    Serial.print(F("Updates per second: "));
    Serial.println(t);
    t = 0;
    Serial.print(F("Loop took: "));
    Serial.print(loopPeriod);
    Serial.println(F(" micros"));
  }
#endif

  loopEndTime = micros();
  // Don't change the measured loop time immediately as it might float around
  loopPeriod = (unsigned long)(loopPeriod * 0.6f) + ((micros() - loopStartTime) * 0.4f);
}

Ball MakeBall(float x, uint8_t y, uint8_t direction, CRGB color)
{
  Ball newBall;
  newBall.x = x;
  newBall.y = y;
  newBall.direction = direction;
  newBall.color = color;
  return newBall;
}

Circle MakeCircle(uint8_t x, uint8_t y, float radius, CRGB color)
{
  Circle newCircle;
  newCircle.x = x;
  newCircle.y = y;
  newCircle.radius = radius;
  newCircle.color = color;
  return newCircle;
}

Raindrop MakeRaindrop(uint8_t x, float y, CRGB color)
{
  Raindrop newRaindrop;
  newRaindrop.x = x;
  newRaindrop.y = y;
  newRaindrop.color = color;
  return newRaindrop;
}

// This function fades the color brightness to the fraction
CRGB ColorFraction(CRGB colorIn, float fraction)
{
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

// This function takes both x and y coordinates and draws the corresponding led mapped to the virtual 2d matrix
void DrawPixel2d(int x, int y, CRGB color)
{
  if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
    leds[WIDTH * y + x] = color;
}

// This function draws a line onto the 1d strip
// Position and length can be float
void DrawLine(float fPos, float length, CRGB color)
{
  // Calculate how much the first pixel will hold
  float availFirstPixel = 1.0f - (fPos - (long)(fPos));
  float amtFirstPixel = min(availFirstPixel, length);
  float remaining = min(length, NUM_LEDS - fPos);
  int iPos = fPos;
  
  // Blend in the color of the first partial pixel
  if (remaining > 0.0f)
  {
    if (0 <= iPos && iPos < NUM_LEDS)
      leds[iPos++] += ColorFraction(color, amtFirstPixel);
    remaining -= amtFirstPixel;
  }
  
  // Now draw every full pixels in the middle
  while (remaining > 1.0f)
  {
    if (0 <= iPos && iPos < NUM_LEDS)
      leds[iPos++] += color;
    remaining--;
  }
  
  // Draw tail pixel, up to a single full pixel
  if (remaining > 0.0f)
  {
    if (0 <= iPos && iPos < NUM_LEDS)
      leds[iPos] += ColorFraction(color, remaining);
  }
}

// This function draws a square onto the virtual 2d matrix
// Coordinate and diameter can be float
void DrawSquare(float fX, float fY, float diameter, CRGB color)
{
  float availFirstPixelX = 1.0f - (fX - (long)(fX));
  float availFirstPixelY = 1.0f - (fY - (long)(fY));
  float amtFirstPixelX = min(availFirstPixelX, diameter);
  float amtFirstPixelY = min(availFirstPixelY, diameter);
  float remainingX = min(diameter, WIDTH - fX);
  float remainingY = min(diameter, HEIGHT - fY);
  int iX = fX;
  int iY = fY;

  // Draw the first row
  if (remainingY > 0.0f)
  {
    // Blend in the color of the first partial pixel of the first row
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += ColorFraction(color, amtFirstPixelX * amtFirstPixelY / 1 * 1);
      remainingX -= amtFirstPixelX;
    }
    
    // Draw every pixels in the middle of the first row
    while (remainingX > 1.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += ColorFraction(color, 1 * amtFirstPixelY / 1 * 1);
      remainingX--;
    }
    
    // Draw the tail pixel of the first row, up to a single full pixel
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX] += ColorFraction(color, remainingX * amtFirstPixelY / 1 * 1);
    }
  }

  // Draw every middle rows
  while (remainingY > 1.0f)
  {
    remainingX = min(diameter, WIDTH - fX);
    iX = fX;
    iY++;
    // Blend in the color of the first partial pixels of the middle rows
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += ColorFraction(color, amtFirstPixelX * 1 / 1 * 1);
      remainingX -= amtFirstPixelX;
    }
    
    // Draw every pixels in the middle of the middle rows
    while (remainingX > 1.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += color;
      remainingX--;
    }
    
    // Draw the tail pixels of the middle rows
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
      leds[WIDTH * iY + iX] += ColorFraction(color, remainingX * 1 / 1 * 1);
    }
    remainingY--;
  }

  // Finally, draw the last row
  if (remainingY > 0.0f)
  {
    remainingX = min(diameter, WIDTH - fX);
    iX = fX;
    iY++;
    // Blend in the color of the first partial pixel of the last row
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += ColorFraction(color, remainingX * remainingY / 1 * 1);
      remainingX -= amtFirstPixelX;
    }
    
    // Draw every pixels in the middle of the last row
    while (remainingX > 1.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += ColorFraction(color, 1 * remainingY / 1 * 1);
      remainingX--;
    }
    
    // Draw the tail pixel of the last row
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX] += ColorFraction(color, remainingX * remainingY / 1 * 1);
    }
  }
}

void DrawCircle_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color)
{
    DrawPixel2d(xc+x, yc+y, color);
    DrawPixel2d(xc-x, yc+y, color);
    DrawPixel2d(xc+x, yc-y, color);
    DrawPixel2d(xc-x, yc-y, color);
    DrawPixel2d(xc+y, yc+x, color);
    DrawPixel2d(xc-y, yc+x, color);
    DrawPixel2d(xc+y, yc-x, color);
    DrawPixel2d(xc-y, yc-x, color);
}

// https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
void DrawCircle(uint8_t xc, uint8_t yc, uint8_t r, CRGB color)
{
    uint8_t x = 0, y = r;
    uint8_t d = 3 - 2 * r;
    DrawCircle_internal(xc, yc, x, y, color);
    while (y >= x)
    {
        // for each point we will
        // draw all eight pixels
        x++;
        // check for decision parameter
        // and correspondingly
        // update d, x, y
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        DrawCircle_internal(xc, yc, x, y, color);
    }
}

void NextRgbState()
{
  switch (rgbState)
  {
    case staticLight:
      rgbState = rainbow;
      break;
    case rainbow:
      rgbState = spreadOut;
      break;
    case spreadOut:
      rgbState = breathing;
      break;
    case breathing:
      rgbState = fractionalDrawingTest2d;
      break;
    case fractionalDrawingTest2d:
      rgbState = spinningRainbow;
      break;
    case spinningRainbow:
      rgbState = ripple;
      break;
    case ripple:
      rgbState = antiRipple;
      break;
    case antiRipple:
      rgbState = stars;
      break;
    case stars:
      rgbState = raindrop;
      break;
    default:
      rgbState = staticLight;
      break;
  }
}

void UpdateEffect()
{
  static unsigned long lastEffectUpdate = 0ul;
  static float secondsElapsed = 0.0f;
  static uint8_t rainbowState = 0;
  static uint8_t breathingState = 0;
  static float breathingStateElapsed = 0.0f;
  static const uint8_t breathingRainbowHues[7] = {0,32,64,96,160,176,192};
  static uint8_t spinningRainbowState = 0;
  static float starsDelayElapsed = 0.0f;
  static float raindropDelayElapsed = 0.0f;
#ifdef Debug
  static unsigned long lastEffectDebug = 0;
#endif
  if (micros() - lastEffectUpdate < 33333 /* 30 fps */) return;
  secondsElapsed = (micros() - lastEffectUpdate) / 1000.0f / 1000.0f;
  lastEffectUpdate = micros();
  
  switch (rgbState)
  {
    case staticLight:
      // ========== Light when pressed ==========

      rainbowState++;
      
      for (uint8_t i = 0; i < HEIGHT; i++)
      {
        for (uint8_t j = 0; j < WIDTH; j++)
        {
          leds[4*i+j] = !btnState[i][j] ? CHSV((4*i+j) * 25 - (rainbowState * 1), 255, rgbBrightness) : CHSV(0, 0, 0);
        }
      }
      
      break;
      // ==============================
    case rainbow:
      // ========== Rainbow ==========
      
      rainbowState++;
      
      for (uint8_t i = 0; i < WIDTH; i++)
      {
        leds[4*0+i] = leds[4*1+i] = leds[4*2+i] = CHSV(i * 10 - (rainbowState * 1), 255, rgbBrightness);
      }
      
      break;
      // ==============================
    case spreadOut:
      // ========== Spread lights out when pressed ==========
      
      // If the last state isn't the same, that means the user just switched to this effect,
      // clear the effect data vector
      if (lastRgbState != spreadOut)
      {
        FastLED.clear();
        balls.clear();
        break;
      }

      FastLED.clear();

      for (auto ball = balls.begin(); ball != balls.end(); )
      {
        ball->x += ball->direction * 10.0f * secondsElapsed;
        
        DrawLine(4 * ball->y + constrain(ball->x, 0.5f, 3.5f) - 0.5f, 1, ball->color);

        if (ball->x < 0.0f || ball->x >= 4.0f)
          balls.erase(ball);
        else
          ball++;
      }

      break;
      // ==============================
    case breathing:
      // ========== Breathing ==========

      breathingStateElapsed += secondsElapsed;
      if (breathingStateElapsed >= 4.0f)
      {
        breathingStateElapsed = 0.0f;
        breathingState += 1;
        if (breathingState > 6)
          breathingState = 0;
      }
      
      for (uint8_t i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CHSV(breathingRainbowHues[breathingState], 255, 15 + (int)(rgbBrightness * 1.5f *
                                                                  (breathingStateElapsed <= 4.0f/2 ?
                                                                  breathingStateElapsed / 2 :
                                                                  (2.0f - (breathingStateElapsed - 2.0f)) / 2) ));
      }
      
      break;
      // ==============================
    case fractionalDrawingTest2d:
      // ========== Fractional drawing test 2D ==========

      FastLED.clear();
      DrawSquare(fractionalDrawingTestX, fractionalDrawingTestY, 1.1, CRGB(CHSV(128, 255, rgbBrightness)));
      
      break;
      // ==============================
    case spinningRainbow:
      // ========== Spinning rainbow ==========
      
      spinningRainbowState++;
      
      FastLED.clear();

      // Todo: Real spinning rainbow
      DrawSquare(0.5f, 0.5f, 1, CRGB(CHSV(spinningRainbowState, 255, rgbBrightness)));
      DrawSquare(2.5f, 0.5f, 1, CRGB(CHSV(spinningRainbowState + 64, 255, rgbBrightness)));
      DrawSquare(0.5f, 1.5f, 1, CRGB(CHSV(spinningRainbowState + 64 * 2, 255, rgbBrightness)));
      DrawSquare(2.5f, 1.5f, 1, CRGB(CHSV(spinningRainbowState + 64 * 3, 255, rgbBrightness)));

      break;
      // ==============================
    case ripple:
      // ========== Water wave ==========

      // If the last state isn't the same, that means the user just switched to this effect,
      // clear the leds, and clear the effect data vector
      if (lastRgbState != ripple)
      {
        FastLED.clear();
        circles.clear();
        break;
      }

      FastLED.clear();

      for (auto circle = circles.begin(); circle != circles.end(); )
      {
        circle->radius += 15.0f * secondsElapsed;
        
        DrawCircle(circle->x, circle->y, circle->radius, circle->color);

        if(circle->radius >= 5.0f)
          circles.erase(circle);
        else
          circle++;
      }

      break;
      // ==============================
    case antiRipple:
      // ========== Anti water wave ==========

      // If the last state isn't the same, that means the user just switched to this effect,
      // clear the leds, and clear the effect data vector
      if (lastRgbState != antiRipple)
      {
        FastLED.clear();
        circles.clear();
        break;
      }

      FastLED.clear();

      for (auto circle = circles.begin(); circle != circles.end(); )
      {
        circle->radius -= 15.0f * secondsElapsed;
        
        if (circle->radius > 0.0f)
          DrawCircle(circle->x, circle->y, circle->radius, circle->color);

        if(circle->radius <= 0.0f)
          circles.erase(circle);
        else
          circle++;
      }

      break;
      // ==============================
    case stars:
      // ========== Stars ==========

      // If the last state isn't the same, that means the user just switched to this effect,
      // init the delayElapsed, and init all the buttons to random rainbow colors
      if (lastRgbState != stars)
      {
        starsDelayElapsed = 0.0f;
        for (uint8_t i = 0; i < NUM_LEDS; i++)
          leds[i] = CHSV(breathingRainbowHues[random(7)], 255, rgbBrightness);
        break;
      }

      starsDelayElapsed += secondsElapsed;

      if (starsDelayElapsed >= 0.1f)
      {
        starsDelayElapsed = 0.0f;
        leds[random(NUM_LEDS)] = CHSV(breathingRainbowHues[random(7)], 255, rgbBrightness);
      }

      break;
      // ==============================
    case raindrop:
      // ========== Raindrop ==========

      // If the last state isn't the same, that means the user just switched to this effect,
      // init the delayElapsed, and clear the effect data vector
      if (lastRgbState != raindrop)
      {
        raindropDelayElapsed = 0.0f;
        raindrops.clear();
      }

      raindropDelayElapsed += secondsElapsed;

      if (raindropDelayElapsed >= 0.3f)
      {
        raindropDelayElapsed = 0.0f;
        raindrops.push_back(MakeRaindrop(random(WIDTH), 0, CRGB(CHSV(random(256), 255, rgbBrightness))));
      }

      FastLED.clear();

      for (auto raindrop = raindrops.begin(); raindrop != raindrops.end(); )
      {
        raindrop->y += 4.0f * secondsElapsed;
        
        DrawSquare(raindrop->x, raindrop->y, 1.0f, raindrop->color);

        if(raindrop->y >= HEIGHT)
          raindrops.erase(raindrop);
        else
          raindrop++;
      }

      break;
      // ==============================
  }

  lastRgbState = rgbState;
#ifdef Debug
  if (millis() - lastEffectDebug > 1000)
  {
    lastEffectDebug = millis();
    Serial.print(F("Effect update took: "));
    Serial.print(micros() - lastEffectUpdate);
    Serial.println(F(" micros"));
  }
#endif
}

void UpdateRgb()
{
  static unsigned long lastRgbUpdate = 0ul;
  if (micros() - lastRgbUpdate < 33333 /* 30 fps */) return;
  lastRgbUpdate = micros();
  FastLED.show();
}

void UpdateLed()
{
  if (ledState == LOW)
  {
    if (millis() - lastBlinkTime > 1000ul - ledBlinkLen)
    {
      lastBlinkTime = millis();
      ledState = HIGH;
      digitalWrite(LED_BUILTIN, ledState);
    }
  } else {
    if (millis() - lastBlinkTime > ledBlinkLen)
    {
      lastBlinkTime = millis();
      ledState = LOW;
      digitalWrite(LED_BUILTIN, ledState);
    }
  }
}

int usedRam() 
{
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
