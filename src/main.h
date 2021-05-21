#include <Arduino.h>
#include <FastLED.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define WIDTH 4
#define HEIGHT 3
#define RGB_PIN 12
#define NUM_LEDS WIDTH * HEIGHT

enum RgbState {
  staticLight,
  rainbow,
  spreadOut,
  breathing,
  fractionalDrawingTest2d,
  spinningRainbow,
  ripple,
  antiRipple,
  stars,
  raindrop,
  snake,
  whacAMole,
  shootingParticles
};

enum MoleState {
  ready,
  playing,
  score
};

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

typedef struct {
  uint8_t x;
  float y;
  CRGB color;
} Raindrop;

typedef struct {
  float x;
  float y;
  float vX;
  float vY;
  CRGB color;
} Particle;

void setup();
void loop();
void ScanKeys();
Ball MakeBall(float x, uint8_t y, uint8_t direction, CRGB color);
Circle MakeCircle(uint8_t x, uint8_t y, float radius, CRGB color);
Raindrop MakeRaindrop(uint8_t x, float y, CRGB color);
Particle MakeParticle(float x, float y, float vX, float vY, CRGB color);
CRGB ColorFraction(CRGB colorIn, float fraction);
void DrawPixel2d(int x, int y, CRGB color);
void DrawLine(float fPos, float length, CRGB color);
void DrawSquare2d(float fX, float fY, float length, CRGB color);
void DrawCircle2d_internal(uint8_t xc, uint8_t yc, uint8_t x, uint8_t y, CRGB color);
void DrawCircle2d(uint8_t xc, uint8_t yc, uint8_t r, CRGB color);
void NextRgbState();
void UpdateEffect();
void UpdateRgb();
void UpdateLed();
long Random(long max);
int usedRam();