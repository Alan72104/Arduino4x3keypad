#include <Arduino.h>
#include <FastLED.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define WIDTH 4
#define HEIGHT 3
#define LED_PIN 12
#define NUM_LEDS WIDTH * HEIGHT

typedef struct {
  float x;
  uint8_t y;
  int8_t direction;
  CRGB color;
} Ball;

typedef struct {
  uint8_t x;
  uint8_t y;
  float radius;
  CRGB color;
} Circle;

enum RgbState {
  lightWhenPressed,
  rainbow,
  spreadLightsOutWhenPressed,
  breathing,
  fractionalDrawingTest2d,
  spinningRainbow,
  waterWave,
  antiWaterWave
};

void setup();
void loop();
Ball MakeBall(float x, uint8_t y, uint8_t direction, CRGB color);
Circle MakeCircle(uint8_t x, uint8_t y, float radius, CRGB color);
CRGB ColorFraction(CRGB colorIn, float fraction);
void DrawPixel2d(int x, int y, CRGB color);
void DrawLine(float fPos, float length, CRGB color);
void DrawSquare(float fX, float fY, float length, CRGB color);
void DrawCircle(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color);
void CircleBres(uint8_t xc, uint8_t yc, uint8_t r, CRGB color);
void NextRgbState();
void UpdateEffect();
void UpdateRgb();
void UpdateLed();
int usedRam();