#include <stdlib.h>

#include <Adafruit_NeoPixel.h>

#define REACTOR_LED_COUNT 3
#define REACTOR_LED_DATA_PIN 6
#define REACTOR_LED_DEFAULT_BRIGHTNESS 60

#define WEAPON_LED_COUNT 4
#define WEAPON_LED_DATA_PIN 5
#define WEAPON_LED_DEFAULT_BRIGHTNESS 60

#define RED_COLOR 0
#define ORANGE_COLOR 32
#define YELLOW_COLOR 64
#define GREEN_COLOR 127
#define TEAL_COLOR 192
#define BLUE_COLOR 256
#define PURPLE_COLOR 329
#define WHITE_COLOR -1

// Chance for white noise, 1/X
#define WHITE_NOISE_FREQUENCY 7

#define ANIMATION_ARRAY_SIZE 10
// Animation function pointers.
double (*animations[ANIMATION_ARRAY_SIZE]) (Adafruit_NeoPixel pixels, double ms, double info);
// Animation function info.
double animation_info[ANIMATION_ARRAY_SIZE];
uint8_t animations_count;


Adafruit_NeoPixel _reactor = Adafruit_NeoPixel(REACTOR_LED_COUNT, REACTOR_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel _weapon = Adafruit_NeoPixel(WEAPON_LED_COUNT, WEAPON_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);

Adafruit_NeoPixel _pixels = _reactor;

void setup() {
  _reactor.begin();
  _weapon.begin();
  
  _reactor.show();
  _weapon.show();
  
  _reactor.setBrightness(REACTOR_LED_DEFAULT_BRIGHTNESS);
  _weapon.setBrightness(WEAPON_LED_DEFAULT_BRIGHTNESS);

  Serial.begin(9600);
  
  animations_count = 1;
  animations[0] = animation_reactor_1;
  animation_info[0] = 0.0;
}

void loop() {
  
  animation_loop();
    
  // full_color(_weapon, RED_COLOR);
  // show(_weapon);
  // 
  // for (int i = 0; i < 15; i++) {
  //   animate_backgroundColorWithColorFillingInForward(_reactor, PURPLE_COLOR, GREEN_COLOR, 40);
  //   animate_backgroundColorWithColorFillingInForward(_reactor, GREEN_COLOR, PURPLE_COLOR, 40);
  // }
}

void animation_loop() {
  for (uint8_t i = 0; i < animations_count; i++) {
    animation_info[i] = (*animations[i])(_reactor, millis(), animation_info[i]);
  }
  
  delay(10);
  

  // double (*anim)(double, double);
  // anim = &animation_reactor_1;
  // v = (*anim)(1.0, v);
  
}

double animation_reactor_1(Adafruit_NeoPixel pixels, double current_ms, double info) {
  double const animation_length_ms = 100;
  
  // First run.
  if (info == 0) {
    info = current_ms + animation_length_ms;
  }
  
  NOT QUITE!
  
  double end_time_ms = info;
  double animation_elapsed_ms = end_time_ms - current_ms;
  
  if (animation_elapsed_ms >= animation_length_ms) {
    return 0.0;
  }  else {
    double percentage = animation_elapsed_ms / animation_length_ms;
    Serial.print("h ");
    Serial.println(percentage);
  }
  
  return end_time_ms;
}

double empty_animation(Adafruit_NeoPixel pixels, double ms, double info) {
  // Do nothing.
}







void animate_backgroundColorWithColorFillingInForward(Adafruit_NeoPixel pixels, uint16_t background, uint16_t fill, uint16_t wait) {
  full_color(pixels, background);
  show(pixels);
  delay(wait);

  for (int16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color(fill));
    pixels.show();
    whiteNoise(pixels, WHITE_NOISE_FREQUENCY, (rand() % 3) + 1);
    delay(wait);
  }
}

void animate_backgroundColorWithColorFillingInBackward(Adafruit_NeoPixel pixels, uint16_t background, uint16_t fill, uint16_t wait) {
  full_color(pixels, background);
  show(pixels);
  delay(wait);

  for (int16_t i = pixels.numPixels() - 1; i >= 0; i--) {
    pixels.setPixelColor(i, color(fill));
    pixels.show();
    whiteNoise(pixels, WHITE_NOISE_FREQUENCY, (rand() % 3) + 1);
    delay(wait);
  }
}

void whiteNoise(Adafruit_NeoPixel pixels, uint16_t chance, uint16_t number_to_light) {
  // srand(millis());
  if (rand() % chance == 0) {
    for (uint16_t i = 0; i < number_to_light; i++) {
      uint16_t pixel = rand() % pixels.numPixels();
      pixels.setPixelColor(pixel, color(WHITE_COLOR));
    }
  }
}

// void rainbowCycle(uint8_t wait) {
//   uint16_t i, j;
// 
//   for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//     for(i=0; i< LED_COUNT; i++) {
//       _pixels.setPixelColor(i, color(((i * 256 / LED_COUNT) + j) & 255));
//     }
//     _pixels.show();
//     delay(wait);
//   }
// }

void full_color(Adafruit_NeoPixel pixels, uint16_t c) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color(c));
  }
}

void full_off(Adafruit_NeoPixel pixels) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, 0);
  }
}

void show(Adafruit_NeoPixel pixels) {
  pixels.show(); 
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
