#include <stdlib.h>

#include <Adafruit_NeoPixel.h>

#define LED_COUNT 10
#define LED_DATA_PIN 9
#define LED_DEFAULT_BRIGHTNESS 30

#define RED_COLOR 0
#define ORANGE_COLOR 32
#define YELLOW_COLOR 64
#define GREEN_COLOR 127
#define TEAL_COLOR 192
#define BLUE_COLOR 256
#define PURPLE_COLOR 329
#define WHITE_COLOR -1

Adafruit_NeoPixel _pixels = Adafruit_NeoPixel(LED_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ400);

void setup() {
  _pixels.begin();
  _pixels.show();
  _pixels.setBrightness(LED_DEFAULT_BRIGHTNESS);
  Serial.begin(9600);
}

void loop() {
  //rainbowCycle(20);
  
  animate_backgroundColorWithColorFillingInForward(TEAL_COLOR, YELLOW_COLOR, 50);
  animate_backgroundColorWithColorFillingInBackward(YELLOW_COLOR, TEAL_COLOR, 50);
}

void animate_backgroundColorWithColorFillingInForward(uint16_t background, uint16_t fill, uint16_t wait) {
  full_color(background);
  _pixels.show();
  delay(wait);

  for (int16_t i = 0; i < LED_COUNT; i++) {
    _pixels.setPixelColor(i, color(fill));
    _pixels.show();
    whiteNoise(10, 1);
    delay(wait);
  }
}

void animate_backgroundColorWithColorFillingInBackward(uint16_t background, uint16_t fill, uint16_t wait) {
  full_color(background);
  _pixels.show();
  delay(wait);

  for (int16_t i = LED_COUNT - 1; i >= 0; i--) {
    _pixels.setPixelColor(i, color(fill));
    _pixels.show();
    whiteNoise(10, 1);
    delay(wait);
  }
}

void whiteNoise(uint16_t chance, uint16_t pixels) {
  srand(millis());
  if (rand() % chance == 0) {
    for (uint16_t i = 0; i < pixels; i++) {
      uint16_t pixel = rand() % LED_COUNT;
      _pixels.setPixelColor(pixel, color(WHITE_COLOR));
    }
  }
}

void animate_blueWithYellow(uint16_t wait) {
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    full_color(BLUE_COLOR);
    _pixels.setPixelColor(i, color(YELLOW_COLOR));
    _pixels.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< LED_COUNT; i++) {
      _pixels.setPixelColor(i, color(((i * 256 / LED_COUNT) + j) & 255));
    }
    _pixels.show();
    delay(wait);
  }
}

void full_color(uint16_t c) {
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    _pixels.setPixelColor(i, color(c));
  }
}

void show() {
  _pixels.show(); 
}

// Color 0 from 383
// -1 is white
uint32_t color(int16_t color)  {
  if (color < 0) {
    return _pixels.Color(255, 255, 255);
  }

  byte r, g, b;
  int range = color / 128;
  switch (range) {
    case 0: // Red to Yellow (0 to 127)
      r = 127 - color % 128;
      g = color % 128;
      b = 0;
      break;
    case 1: // Yellow to Teal (128 to 255)
      r = 0;
      g = 127 - color % 128;
      b = color % 128;
      break;
    case 2: // Teal to Purple to Red (256 to 384)
      r = color % 128;
      g = 0;
      b = 127 - color % 128;
      break;
  }
  return _pixels.Color(r, g, b);
}

uint32_t whiteColor() {
  return _pixels.Color(255, 255, 254);
}
