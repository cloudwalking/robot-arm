#include <Adafruit_NeoPixel.h>
#include <stdlib.h>

#include "Animation.h"

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
// double (*animations[ANIMATION_ARRAY_SIZE]) (Adafruit_NeoPixel pixels, double ms, double info);
// Animation function info.
// double animation_info[ANIMATION_ARRAY_SIZE];
Animation animations[ANIMATION_ARRAY_SIZE];
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

  // The first animation.
  Animation first_animation;
  first_animation.userInfo = 0.0;
  first_animation.isFinished = false;
  void (*first_animation_fuction)(Animation *, double) = &animation_reactor_PurpleWithGreenFillingForward;
  first_animation.function = first_animation_fuction;

  animations[0] = first_animation;

  animations_count = 1;
}

void loop() {
  animation_loop();
  delay(10);
}

void animation_loop() {
  for (uint8_t i = 0; i < animations_count; i++) {
    Animation *a = &animations[i];
    void (*function)(Animation *, double) = (*a).function;
    function(a, millis());
  }
  show(_reactor);
  show(_weapon);
}

void animation_reactor_PurpleWithGreenFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, PURPLE_COLOR, GREEN_COLOR,
                                                 animation, current_ms);
}


void animate_backgroundColorWithColorFillingForward(Adafruit_NeoPixel pixels, uint16_t back, uint16_t fill,
                                                    Animation *animation, double current_ms) {
  // Animation time in milliseconds.
  double const animation_length_ms = 250.0;

  // First some boilerplate.
  double end_time_ms = animation->userInfo;

  // Is this the first run?
  if (end_time_ms == 0.0) {
    end_time_ms = current_ms + animation_length_ms;
    animation->userInfo = end_time_ms;
  }
  // Expired animation?
  else if (end_time_ms < current_ms) {
    // If this animation has expired, reset it.
    animation->userInfo = 0.0;
    animation->isFinished = true;
  }
  
  double remaining_ms = end_time_ms - current_ms;
  double percentage_remaining = 100 * remaining_ms / animation_length_ms;
  double percentage_complete = 100 - percentage_remaining;
  
  // Now push some pixels!!

  // Is this the first run?
  if (percentage_complete == 100.0) {
    full_color(pixels, back);
  }
  
  // This animation fills in LEDs from left to right. Choose the furthest right pixel.
  int right_pixel = percentage_complete / (100.0 / pixels.numPixels());
  
  Serial.print("percentage : "); Serial.println(percentage_complete);
  Serial.print("right_pixel: "); Serial.println(right_pixel);
  
  // Fill in everything to the left.
  for (int16_t i = 0; i < right_pixel; i++) {
    pixels.setPixelColor(i, color(fill));
  }
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

// void animate_backgroundColorWithColorFillingInBackward(Adafruit_NeoPixel pixels, uint16_t background, uint16_t fill, uint16_t wait) {
//   full_color(pixels, background);
//   show(pixels);
//   delay(wait);
// 
//   for (int16_t i = pixels.numPixels() - 1; i >= 0; i--) {
//     pixels.setPixelColor(i, color(fill));
//     pixels.show();
//     whiteNoise(pixels, WHITE_NOISE_FREQUENCY, (rand() % 3) + 1);
//     delay(wait);
//   }
// }
// 
// void whiteNoise(Adafruit_NeoPixel pixels, uint16_t chance, uint16_t number_to_light) {
//   // srand(millis());
//   if (rand() % chance == 0) {
//     for (uint16_t i = 0; i < number_to_light; i++) {
//       uint16_t pixel = rand() % pixels.numPixels();
//       pixels.setPixelColor(pixel, color(WHITE_COLOR));
//     }
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
