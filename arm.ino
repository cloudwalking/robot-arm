#include <Adafruit_NeoPixel.h>
#include <stdlib.h>

#include "Animation.h"

#define REACTOR_LED_COUNT 3
#define REACTOR_LED_DATA_PIN 6
#define REACTOR_LED_DEFAULT_BRIGHTNESS 40

#define WEAPON_LED_COUNT 5
#define WEAPON_LED_DATA_PIN 5
#define WEAPON_LED_DEFAULT_BRIGHTNESS 20

#define RED_COLOR 0
#define ORANGE_COLOR 32
#define YELLOW_COLOR 64
#define GREEN_COLOR 127
#define TEAL_COLOR 192
#define BLUE_COLOR 256
#define PURPLE_COLOR 329
#define WHITE_COLOR -1
#define OFF_COLOR -2

// Chance for white noise, 1/X
#define WHITE_NOISE_FREQUENCY 7

#define ANIMATION_ARRAY_SIZE 10
Animation* animations[ANIMATION_ARRAY_SIZE];
uint8_t animations_count;

Adafruit_NeoPixel _reactor = Adafruit_NeoPixel(REACTOR_LED_COUNT, REACTOR_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel _weapon = Adafruit_NeoPixel(WEAPON_LED_COUNT, WEAPON_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);

Adafruit_NeoPixel _pixels = _reactor;

void setup() {
  animations_count = 0;
  
  Serial.begin(9600);
  // while (!Serial) { }

  _reactor.begin();
  _weapon.begin();
  
  _reactor.setBrightness(REACTOR_LED_DEFAULT_BRIGHTNESS);
  _weapon.setBrightness(WEAPON_LED_DEFAULT_BRIGHTNESS);

  static Animation reactor1 = animations_newAnimation();
  reactor1.animationDuration = 400.0;
  reactor1.function = &animation_reactor1;

  animations_addAnimation(&reactor1);
  
  static Animation weapon1 = animations_newAnimation();
  weapon1.animationDuration = 6000.0;
  weapon1.function = &animation_weapon1;
  
  animations_addAnimation(&weapon1);
}

void loop() {
  animations_loop();
  delay(20);
}

void animations_loop() {
  for (uint8_t i = 0; i < animations_count; i++) {
    Animation *a = animations[i];
    void (*function)(Animation *, double) = (*a).function;
    function(a, millis());
  }
  show(_reactor);
  show(_weapon);
}

// This animation is two keyframes, each of which is its own animation.
// First keyframe is purple being filled by green.
// Second keyframe is green being filled by purple.
void animation_reactor1(Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  static Animation purple_filling_with_green;
  static Animation green_filling_with_purple;

  // New animation?
  if (percentage_complete == 0.0) {
    purple_filling_with_green.animationDuration = animation->animationDuration / 2;
    green_filling_with_purple.animationDuration = animation->animationDuration / 2;
    
    purple_filling_with_green.userInfo = 0.0;
    green_filling_with_purple.userInfo = 0.0;
    
    // We will start these up when appropriate.
    purple_filling_with_green.isFinished = true;
    green_filling_with_purple.isFinished = true;
    
    void (*purple_filling_with_green_fn)(Animation *, double) = &animation_reactor_PurpleWithGreenFillingForward;
    void (*green_filling_with_purple_fn)(Animation *, double) = &animation_reactor_GreenWithPurpleFillingForward;

    purple_filling_with_green.function = purple_filling_with_green_fn;
    green_filling_with_purple.function = green_filling_with_purple_fn;
    
    animations_addAnimation(&purple_filling_with_green);
    animations_addAnimation(&green_filling_with_purple);
  }
  
  if (percentage_complete < 50.0) {
    // Enable purple being filled by green.
    purple_filling_with_green.isFinished = false;
    // Disable green being filled by purple.
    green_filling_with_purple.isFinished = true;
  } else {
    // Disable purple being filled by green.
    purple_filling_with_green.isFinished = true;
    // Enable green being filled by purple.
    green_filling_with_purple.isFinished = false;
  }
}

void animation_weapon1(Animation *animation, double current_ms) {
  // Loop forever
  animation->isFinished = false;

  animate_backgroundColorWithColorFillingForward(_weapon, OFF_COLOR, RED_COLOR,
                                                 animation, current_ms);
}

void animation_reactor_PurpleWithGreenFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, PURPLE_COLOR, GREEN_COLOR,
                                                 animation, current_ms);
  whiteNoise(_reactor, 80, (rand() % 3) + 1);
  whiteNoise(_reactor, 100, (rand() % 3) + 1);
}

void animation_reactor_GreenWithPurpleFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, GREEN_COLOR, PURPLE_COLOR,
                                                animation, current_ms);
  whiteNoise(_reactor, 80, (rand() % 3) + 1);
  whiteNoise(_reactor, 100, (rand() % 3) + 1);
}

// This is an animation where the background is filled with a solid color, and a second color animates in across it.
// First keyframe is all background color.
// Second keyframe is first pixel colored.
// Third keyframe is second pixel colored.
// Etc.
void animate_backgroundColorWithColorFillingForward(Adafruit_NeoPixel pixels, uint16_t back, uint16_t fill,
                                                    Animation *animation, double current_ms) {
  if (animation->isFinished) {
    return;
  }

  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  // Now push some pixels!
  
  // Is this the first run?
  // if (percentage_complete == 0.0) {
  //   full_color(pixels, back);
  // }
  
  // First keyframe is all background color.
  // Second keyframe is first pixel colored.
  // Third keyframe is second pixel colored.
  // Etc.
  double const numberOfKeyframes = 1 + pixels.numPixels();
  
  // This animation fills in LEDs from left to right. Choose the furthest right pixel.
  int right_pixel = percentage_complete / (100.0 / numberOfKeyframes);
  
  // Fill in everything up to the right.
  for (int16_t i = 0; i < right_pixel; i++) {
    pixels.setPixelColor(i, color(fill));
  }
  
  // Fill in everything else.
  for (uint16_t i = right_pixel; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, color(back));
  }
}

void whiteNoise(Adafruit_NeoPixel pixels, uint16_t chance, uint16_t number_to_light) {
  // srand(millis());
  if (rand() % chance == 0) {
    for (uint16_t i = 0; i < number_to_light; i++) {
      uint16_t pixel = rand() % pixels.numPixels();
      pixels.setPixelColor(pixel, color(WHITE_COLOR));
      Serial.println("LIGHTING UP");
      Serial.println();
    }
  }
}

//////////////////////////////
//////////////////////////////
// Animation helper functions
//////////////////////////////
//////////////////////////////

// Creates new animation struct with the given animation function.
// For some reason I can't accept this function pointer as an argument.
// Animation animations_newAnimation(void (*animation_function)(Animation *, double)) {
Animation animations_newAnimation() {
  Animation a;
  a.userInfo = 0.0;
  a.isFinished = false;
  // a.function = animation_function;

  return a;
}

// Run this in the beginning of all animations.
// Returns the animation percentage given the current time.
double animation_boilerplate(Animation *animation, double current_ms) {
  double const animation_length_ms = animation->animationDuration;
  double end_time_ms = animation->userInfo;

  // Is this the first run?
  if (end_time_ms == 0.0) {
    end_time_ms = current_ms + animation_length_ms;
    animation->userInfo = end_time_ms;
  }
  // Expired animation?
  else if (end_time_ms <= current_ms) {
    // Reset this animation for next time.
    animation->userInfo = 0.0;
    // Mark that we're done.
    animation->isFinished = true;
  }
  
  double remaining_ms = end_time_ms - current_ms;
  double percentage_remaining = 100 * remaining_ms / animation_length_ms;
  double percentage_complete = 100 - percentage_remaining;

  return percentage_complete;
}

// Adds an animation. Does nothing if the animations is already on the animation stack.
void animations_addAnimation(Animation *a) {
  for (uint8_t i = 0; i < animations_count; i++) {
    Animation *x = animations[i];
    if (a == x) {
      // Animation is already stored.
      return;
    }
  }

  animations[++animations_count - 1] = a;
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
  if (color == -1) {
    return _pixels.Color(255, 255, 255);
  }
  else if (color == -2) {
    return 0;
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
