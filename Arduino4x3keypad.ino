#include <FastLED.h>
#include <Vector.h>

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
  float pos;
  int row;
  int direction;
  CRGB color;
//  Ball();
//  Ball(int p, int r, int d, CRGB c) : pos(p), row(r), direction(d), color(c) {}
};

CRGB leds[NUM_LEDS];
const int rgbBrightness = 63;
unsigned long lastRgbStateChange = 0ul;
enum RgbState
{
  lightWhenPressed,
  rainbow,
  spreadLightsOutWhenPressed,
  breathing
};
RgbState rgbState = lightWhenPressed;
Ball ball_array[50];
Vector<Ball> balls(ball_array);


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
//  for (int i = 0; i < NUM_LEDS; i++) {
//      leds[i] = CHSV(i * 10 - (j * 1), 255, 255);
//      leds[i] = CRGB::White;
//  }
//  FastLED.show();

  while(!Serial){}
  Serial.begin(19200);
  Serial.println("");
}

//int t;
//unsigned long tt;
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
        if (Serial.availableForWrite())
        {
          Serial.write((4*i+j+1 << 4 ) + (btnState[i][j] == LOW ? 1 : 0));
        }
        if (btnState[2][0] == !HIGH && btnState[0][3] == !HIGH && millis() - lastRgbStateChange >= 500)
        {
          NextRgbState();
        }
        if (rgbState == spreadLightsOutWhenPressed)
        {
          struct Ball newBall1, newBall2;
          newBall1.row = newBall2.row = i;
          newBall1.pos = newBall2.pos = j + 0.5f;
          newBall1.direction = -1;
          newBall2.direction = 1;
          newBall1.color = newBall2.color = CHSV(rand() % 255, 255, rgbBrightness);
          balls.push_back(newBall1);
          balls.push_back(newBall2);
//          CRGB color = CHSV(rand() % 255, 255, rgbBrightness);
//          balls.push_back(Ball(i, j, -1, color));
//          balls.push_back(Ball(i, j, 1, color));
        }
      }
    }
    digitalWrite(pinR[i], HIGH);
    pinMode(pinR[i], INPUT);
  }

//   Update frequency test
//  t++;
//  if (millis() - tt >= 1000)
//  {
//    tt = millis();
//    Serial.print("Updates per second: ");
//    Serial.println(t);
//    t = 0;
//    Serial.print("Microseconds taken for the loop: ");
//    Serial.println(loopPeriod);
//  }


  loopEndTime = micros();
  loopPeriod = micros() - loopStartTime;
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
      rgbState = lightWhenPressed;
      break;
  }
}

CRGB ColorFraction(CRGB colorIn, float fraction)
{
  fraction = min(1.0f, fraction);
  return CRGB(colorIn).fadeToBlackBy(255 * (1.0f - fraction));
}

void DrawPixels(float fPos, float count, CRGB color)
{
  // Calculate how much the first pixel will hold
  float availFirstPixel = 1.0f - (fPos - (long)(fPos));
  float amtFirstPixel = min(availFirstPixel, count);
  float remaining = min(count, NUM_LEDS -fPos);
  int iPos = fPos;
  // Blend (add) in the color of the first partial pixel
  if (remaining > 0.0f)
  {
    leds[iPos++] += ColorFraction(color, amtFirstPixel);
    remaining -= amtFirstPixel;
  }
  // Now draw any full pixels in the middle
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

void UpdateEffect()
{
  static unsigned long lastEffectUpdate = 0ul;
  static float secondsElapsed = 0.0f;
  static int rainbowState = 0;
  static int breathingState = 0;
  static float breathingStateElapsed = 0.0f;
  static const int breathingRainbowHues[7] = {0,32,64,96,160,176,192};
  if (micros() - lastEffectUpdate < 33333 /* 30 fps */) return;
  secondsElapsed = (micros() - lastEffectUpdate) / 1000.0f / 100.0f;
  lastEffectUpdate = micros();
  
  switch (rgbState)
  {
    case lightWhenPressed:
      // ========== Light when pressed ==========

      if (++rainbowState == 255) {rainbowState = 0;}
      
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
      
      if (++rainbowState == 255) {rainbowState = 0;}
      
      for (int i = 0; i < WIDTH; i++)
      {
        leds[4*0+i] = leds[4*1+i] = leds[4*2+i] = CHSV(i * 10 - (rainbowState * 1), 255, rgbBrightness);
      }
      
      break;
      // ==============================
    case spreadLightsOutWhenPressed:
      // ========== Spread lights out when pressed ==========
      
      for (int i = 0; i < NUM_LEDS; i++) 
        leds[i] = 0x000000;
        
      for (int i = 0, deleteCount = 0; i < balls.size(); i++)
      {
        struct Ball *ball = &balls[i - deleteCount];
        
        ball->pos = constrain(ball->pos + ball->direction * 0.2f * secondsElapsed, 0.5f, 3.5f);
          
        DrawPixels(4 * ball->row + ball->pos - 0.5f, 1, ball->color);
        
        if(ball->pos <= 0.5f || ball->pos >= 3.5f)
          balls.remove(i - deleteCount++);
      }
        
      break;
      // ==============================
    case breathing:
      // ========== Breathing ==========

      breathingStateElapsed += secondsElapsed;
      if (breathingStateElapsed >= 4.0f)
      {
        breathingStateElapsed = 0.0f
        breathingState += 1;
        if (breathingState > 6) // Rainbow has 7 colors
          breathingState = 0
      }
      
      // ==============================
  }
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
unsigned long c(unsigned long i)
{
  Serial.println(i);
  return i;
}
