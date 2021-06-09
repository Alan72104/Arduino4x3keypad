#include <Arduino.h>
#include <FastLED.h>
#include "KeypadParams.h"
#include "Rgb.h"

void Rgb::Init()
{
    FastLED.addLeds<WS2812B, RGB_PIN, GRB>(leds, NUM_LEDS);
}

// This function fades the color brightness to the fraction
CRGB Rgb::GetColorFraction(CRGB colorIn, float fraction)
{
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

// This function takes both x and y coordinates and draws the corresponding led mapped to the virtual 2d matrix
void Rgb::DrawPixel2d(int x, int y, CRGB color)
{
  if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT)
    leds[WIDTH * y + x] = color;
}

// This function draws a line onto the 1ed strip
// Position and length can be float
void Rgb::DrawLine(float fPos, float length, CRGB color)
{
  // Calculate how much the first pixel will hold
  float availFirstPixel = 1.0f - (fPos - (uint8_t)(fPos));
  float amtFirstPixel = min(availFirstPixel, length);
  float remaining = min(length, NUM_LEDS - fPos);
  int iPos = fPos;
  
  // Blend in the color of the first partial pixel
  if (remaining > 0.0f)
  {
    if (0 <= iPos && iPos < NUM_LEDS)
      leds[iPos++] += GetColorFraction(color, amtFirstPixel);
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
      leds[iPos] += GetColorFraction(color, remaining);
  }
}

// This function draws a square onto the virtual 2d matrix
// Coordinate and diameter can be float
void Rgb::DrawSquare2d(float fX, float fY, float diameter, CRGB color)
{
  float availFirstPixelX = 1.0f - (fX - (int)fX);
  float availFirstPixelY = 1.0f - (fY - (int)fY);
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
        leds[WIDTH * iY + iX++] += GetColorFraction(color, amtFirstPixelX * amtFirstPixelY / 1 * 1);
      remainingX -= amtFirstPixelX;
    }
    
    // Draw every pixels in the middle of the first row
    while (remainingX > 1.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += GetColorFraction(color, 1 * amtFirstPixelY / 1 * 1);
      remainingX--;
    }
    
    // Draw the tail pixel of the first row, up to a single full pixel
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX] += GetColorFraction(color, remainingX * amtFirstPixelY / 1 * 1);
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
        leds[WIDTH * iY + iX++] += GetColorFraction(color, amtFirstPixelX * 1 / 1 * 1);
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
      leds[WIDTH * iY + iX] += GetColorFraction(color, remainingX * 1 / 1 * 1);
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
        leds[WIDTH * iY + iX++] += GetColorFraction(color, remainingX * remainingY / 1 * 1);
      remainingX -= amtFirstPixelX;
    }
    
    // Draw every pixels in the middle of the last row
    while (remainingX > 1.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX++] += GetColorFraction(color, 1 * remainingY / 1 * 1);
      remainingX--;
    }
    
    // Draw the tail pixel of the last row
    if (remainingX > 0.0f)
    {
      if (0 <= iX && iX < WIDTH && 0 <= iY && iY < HEIGHT)
        leds[WIDTH * iY + iX] += GetColorFraction(color, remainingX * remainingY / 1 * 1);
    }
  }
}

void Rgb::DrawCircle2d_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color)
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
void Rgb::DrawCircle2d(uint8_t xc, uint8_t yc, uint8_t r, CRGB color)
{
    uint8_t x = 0, y = r;
    uint8_t d = 3 - 2 * r;
    DrawCircle2d_internal(xc, yc, x, y, color);
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
        DrawCircle2d_internal(xc, yc, x, y, color);
    }
}

void Rgb::Draw()
{
  static uint32_t lastRgbUpdate = 0ul;
  if (micros() - lastRgbUpdate < 33333 /* 30 fps */) return;
  lastRgbUpdate = micros();
  FastLED.show();
}