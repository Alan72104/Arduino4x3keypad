#include <Arduino.h>
#include <FastLED.h>
#include <vector>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define WIDTH 4
#define HEIGHT 3
#define LED_PIN 12
#define NUM_LEDS WIDTH * HEIGHT

const int pinC[WIDTH] = {8, 7, 6, 5};
const int pinR[HEIGHT] = {2, 3, 4};
int btnStateTemp = LOW;
int btnState[HEIGHT][WIDTH] = {};
int ledState = LOW;
unsigned long lastBlinkTime = 0ul;
unsigned long ledBlinkLen = 5ul;
unsigned long loopEndTime = 0ul;
unsigned long loopStartTime = 0ul;
unsigned long loopPeriod = 0ul;
const int scanPerSec = 1000;
const unsigned long microsPerScan = (unsigned long)(1000000 / scanPerSec);

struct Ball
{
  float x;
  int y;
  int direction;
  CRGB color;
};
struct Circle
{
  int x;
  int y;
  float radius;
  CRGB color;
};

CRGB leds[NUM_LEDS];
int rgbBrightness = 63;
unsigned long lastRgbStateChange = 0ul;
unsigned long lastRgbBrightnessChange = 0ul;
enum RgbState
{
  lightWhenPressed,
  rainbow,
  spreadLightsOutWhenPressed,
  breathing,
  fractionalDrawingTest2d,
  spinningRainbow,
  waterWave
};
RgbState rgbState = lightWhenPressed;
std::vector<Ball> balls(20);
std::vector<Circle> circles(20);

float fractionalDrawingTestY = 0.0f;
float fractionalDrawingTestX = 0.0f;

void setup();
void loop();
Ball MakeBall(float x, int y, int direction, CRGB color);
Circle MakeCircle(int x, int y, float radius, CRGB color);
CRGB ColorFraction(CRGB colorIn, float fraction);
void DrawPixel2d(int x, int y, CRGB color);
void DrawLine(float fPos, float length, CRGB color);
void DrawSquare(float fX, float fY, float length, CRGB color);
void DrawCircle(int xc, int yc, int x, int y, CRGB color);
void CircleBres(int xc, int yc, int r, CRGB color);
void NextRgbState();
void UpdateEffect();
void UpdateRgb();
void UpdateLed();
int c(int i);
float c(float i);
unsigned long c(unsigned long i);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i : pinC)
  {
    pinMode(i, INPUT_PULLUP);
  }
  for (int i : pinR)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  while(!Serial){}
  Serial.begin(19200);
  Serial.println("");
}

// #define Debug

void loop() {
  loopStartTime = micros();
  UpdateLed();
  UpdateEffect();
  UpdateRgb();
  if (micros() - loopEndTime < (microsPerScan - (loopPeriod > microsPerScan ? microsPerScan : loopPeriod))) return;
  
  for (int i = 0; i < HEIGHT; i++)
  {
    pinMode(pinR[i], OUTPUT);
    digitalWrite(pinR[i], LOW);
    for (int j = 0; j < WIDTH; j++)
    {
      btnStateTemp = digitalRead(pinC[j]);
      if (btnState[i][j] != btnStateTemp)
      {
        btnState[i][j] = btnStateTemp;

#ifndef Debug
        if (Serial.availableForWrite())
          Serial.write(((4*i+j+1) << 4 ) + (btnStateTemp == LOW ? 1 : 0));
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
          case fractionalDrawingTest2d:
            if (btnState[0][0] == !HIGH && fractionalDrawingTestY > 0.0f)         fractionalDrawingTestY -= 0.1f;
            else if (btnState[1][0] == !HIGH && fractionalDrawingTestY < HEIGHT)  fractionalDrawingTestY += 0.1f;
            else if (btnState[1][2] == !HIGH && fractionalDrawingTestX < WIDTH)   fractionalDrawingTestX += 0.1f;
            else if (btnState[1][1] == !HIGH && fractionalDrawingTestX > 0.0f)    fractionalDrawingTestX -= 0.1f;
          case spreadLightsOutWhenPressed:
            if (btnStateTemp == !HIGH)
            {
              CRGB color = CHSV(rand() % 255, 255, rgbBrightness);
              balls.push_back(MakeBall(j, i, -1, color));
              balls.push_back(MakeBall(j, i, 1, color));
            }
          case waterWave:
            if (btnStateTemp == !HIGH)
              circles.push_back(MakeCircle(j, i, 0, CRGB(CHSV(rand() % 255, 255, rgbBrightness))));
        }
      }
    }
    digitalWrite(pinR[i], HIGH);
    pinMode(pinR[i], INPUT);
  }

  // Update frequency test
#ifdef Debug
  static int t;
  static unsigned long tt;
  t++;
  if (millis() - tt >= 1000)
  {
    tt = millis();
    Serial.print(F("Updates per second: "));
    Serial.println(t);
    t = 0;
    Serial.print(F("Microseconds taken for the loop: "));
    Serial.println(loopPeriod);
  }
#endif

  loopEndTime = micros();
  loopPeriod = (unsigned long)(loopPeriod * 0.6f) + ((micros() - loopStartTime) * 0.4f);  // Don't change the measured loop time immediately as it might float around
}

Ball MakeBall(float x, int y, int direction, CRGB color)
{
  struct Ball newBall;
  newBall.x = x;
  newBall.y = y;
  newBall.direction = direction;
  newBall.color = color;
  return newBall;
}

Circle MakeCircle(int x, int y, float radius, CRGB color)
{
  struct Circle newCircle;
  newCircle.x = x;
  newCircle.y = y;
  newCircle.color = color;
  return newCircle;
}

CRGB ColorFraction(CRGB colorIn, float fraction)
{
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

void DrawPixel2d(int x, int y, CRGB color)
{
  if (x < WIDTH && y < HEIGHT)
    leds[WIDTH * y + x] = color;
}

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
    leds[iPos++] += ColorFraction(color, amtFirstPixel);
    remaining -= amtFirstPixel;
  }
  
  // Now draw every full pixels in the middle
  while (remaining > 1.0f)
  {
    leds[iPos++] += color;
    remaining--;
  }
  
  // Draw tail pixel, up to a single full pixel
  if (remaining > 0.0f)
  {
    leds[iPos] += ColorFraction(color, remaining);
  }
}

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

void DrawCircle(int xc, int yc, int x, int y, CRGB color)
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
void CircleBres(int xc, int yc, int r, CRGB color)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;
    DrawCircle(xc, yc, x, y, color);
    while (y >= x)
    {
        // for each pixel we will
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
        DrawCircle(xc, yc, x, y, color);
    }
}

void NextRgbState()
{
  switch (rgbState)
  {
    case lightWhenPressed:
      rgbState = rainbow;
      break;
    case rainbow:
      rgbState = spreadLightsOutWhenPressed;
      break;
    case spreadLightsOutWhenPressed:
      rgbState = breathing;
      break;
    case breathing:
      rgbState = fractionalDrawingTest2d;
      break;
    case fractionalDrawingTest2d:
      rgbState = spinningRainbow;
      break;
    case spinningRainbow:
      rgbState = waterWave;
      break;
    default:
      rgbState = lightWhenPressed;
      break;
  }
}

void UpdateEffect()
{
  static unsigned long lastEffectUpdate = 0ul;
  static float secondsElapsed = 0.0f;
  static int rainbowState = 0;
  static int breathingState = 0;
  static float breathingStateElapsed = 0.0f;
  static const int breathingRainbowHues[7] = {0,32,64,96,160,176,192};
  static int spinningRainbowState = 0;
#ifdef Debug
  static unsigned int updateCount = 0;
#endif
  if (micros() - lastEffectUpdate < 33333 /* 30 fps */) return;
  secondsElapsed = (micros() - lastEffectUpdate) / 1000.0f / 1000.0f;
  lastEffectUpdate = micros();
  
  switch (rgbState)
  {
    case lightWhenPressed:
      // ========== Light when pressed ==========

      if (++rainbowState == 255) rainbowState = 0;
      
      for (int i = 0; i < HEIGHT; i++)
      {
        for (int j = 0; j < WIDTH; j++)
        {
          leds[4*i+j] = !btnState[i][j] ? CHSV((4*i+j) * 25 - (rainbowState * 1), 255, rgbBrightness) : CHSV(0, 0, 0);
        }
      }
      
      break;
      // ==============================
    case rainbow:
      // ========== Rainbow ==========
      
      if (++rainbowState > 255) rainbowState = 0;
      
      for (int i = 0; i < WIDTH; i++)
      {
        leds[4*0+i] = leds[4*1+i] = leds[4*2+i] = CHSV(i * 10 - (rainbowState * 1), 255, rgbBrightness);
      }
      
      break;
      // ==============================
    case spreadLightsOutWhenPressed:
      // ========== Spread lights out when pressed ==========
      
      FastLED.clear();
        
      for (auto ball = balls.begin(); ball != balls.end(); )
      {
        ball->x = constrain(ball->x + ball->direction * 10.0f * secondsElapsed, 0.5f, 3.5f);
          
        DrawLine(4 * ball->y + ball->x - 0.5f, 1, ball->color);
        
        if(ball->x <= 0.5f || ball->x >= 3.5f)
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
        if (breathingState > 6) // Rainbow has 7 colors
          breathingState = 0;
      }
      
      for (int i = 0; i < NUM_LEDS; i++)
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
      
      if (++spinningRainbowState > 255) spinningRainbowState = 0;
      
      FastLED.clear();
      DrawSquare(0.5f, 0.5f, 1, CRGB(CHSV(spinningRainbowState, 255, rgbBrightness)));
      DrawSquare(2.5f, 0.5f, 1, CRGB(CHSV(spinningRainbowState + 64, 255, rgbBrightness)));
      DrawSquare(0.5f, 1.5f, 1, CRGB(CHSV(spinningRainbowState + 64 * 2, 255, rgbBrightness)));
      DrawSquare(2.5f, 1.5f, 1, CRGB(CHSV(spinningRainbowState + 64 * 3, 255, rgbBrightness)));

      break;
      // ==============================
    case waterWave:
      // ========== Water wave ==========

      FastLED.clear();

      for (auto circle = circles.begin(); circle < circles.end(); )
      {
        circle->radius += 1.0f * secondsElapsed;
        
        CircleBres(circle->x, circle->y, (int)(circle->radius), circle->color);

        if(circle->radius >= 5.0f)
          circles.erase(circle);
        else
          circle++;
      }

      break;
      // ==============================
  }
#ifdef Debug
  updateCount++;
  if (updateCount % 10 == 0)
  {
    Serial.print("Effect update took: ");
    Serial.print(micros() - lastEffectUpdate);
    Serial.println(" micros");
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

int c(int i)
{
  Serial.println(i);
  return i;
}
float c(float i)
{
  Serial.println(i);
  return i;
}
unsigned long c(unsigned long i)
{
  Serial.println(i);
  return i;
}
