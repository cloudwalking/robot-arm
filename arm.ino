#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <math.h>

#include "Animation.h"

#define REACTOR_LED_COUNT 3
#define REACTOR_LED_DATA_PIN 6
#define REACTOR_LED_DEFAULT_BRIGHTNESS 40

#define WEAPON_LED_COUNT 10
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

#define DEFAULT_ROOT_ANIMATION_DURATION 30000.0

// Useful macros

#define first_run(percentage_complete) percentage_complete == 0.0
#define last_run(percentage_complete) percentage_complete >= 100.0

//

#define ANIMATION_ARRAY_SIZE 10
Animation* animations[ANIMATION_ARRAY_SIZE];
uint8_t animations_count;

Adafruit_NeoPixel _reactor = Adafruit_NeoPixel(REACTOR_LED_COUNT, REACTOR_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel _weapon = Adafruit_NeoPixel(WEAPON_LED_COUNT, WEAPON_LED_DATA_PIN, NEO_GRB + NEO_KHZ400);

// Generic
Adafruit_NeoPixel _pixels = _reactor;

// Function prototypes, for argument defaults
void animate_backgroundColorWithColorFillingForward(Adafruit_NeoPixel pixels, uint16_t back, uint16_t fill,
    Animation *animation, double current_ms, int8_t lowRange = -1, int8_t highRange = -1);
uint32_t color(int16_t color, float brightness = 1.0);

void setup() {
  randomSeed(analogRead(A0));

  animations_count = 0;
  
  Serial.begin(9600);
  // while (!Serial) { }

  setupNeopixels();
  
  // Add the root animation. When this animation starts,
  // it randomly transforms into one of a few different animations.
  
  static Animation root = animations_newAnimation();
  root.duration = DEFAULT_ROOT_ANIMATION_DURATION;
  root.function = &animation_root;

  animations_addAnimation(&root);
}

void setupNeopixels() {
  _reactor.begin();
  _weapon.begin();
  
  _reactor.setBrightness(REACTOR_LED_DEFAULT_BRIGHTNESS);
  _weapon.setBrightness(WEAPON_LED_DEFAULT_BRIGHTNESS);
}

void loop() {
  animations_loop();
  delay(20);
}

void animations_loop() {
  animations_sortAnimations();
  for (uint8_t i = 0; i < animations_count; i++) {
    Animation *a = animations[i];
    void (*function)(Animation *, double) = a->function;
    function(a, millis());
  }
  show(_reactor);
  show(_weapon);
}

void animation_root(Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  if (last_run(percentage_complete)) {
    // Remove all other animations from the stack.
    animations_count = 1;
    // Reset our duration.
    animation->duration = DEFAULT_ROOT_ANIMATION_DURATION;
  }
  
  if (first_run(percentage_complete)) {
    int dice = random() % 6;
    switch (dice) {
      
      case 0:
      case 1: { // Two runs because we have two color options

        static Animation spin = animations_newAnimation();
        spin.duration = 400.0;
        if (random() % 2) spin.function = &animation_reactor_spin_GREEN;
        else              spin.function = &animation_reactor_spin_RED;
        animations_addAnimation(&spin);
        
        static Animation noise = animations_newAnimation();
        noise.duration = 500.0;
        noise.function = &animation_noise_fullsystem;
        noise.zIndex = 10;
        animations_addAnimation(&noise);
        
        static Animation wipe = animations_newAnimation();
        wipe.duration = 500.0;
        wipe.function = &animation_weapon_off;
        animations_addAnimation(&wipe);

      } break;

      case 2: {
        
        static Animation twinkling = animations_newAnimation();
        twinkling.duration = 90.0;
        twinkling.function = &animation_twinkle_TEAL;
    
        animations_addAnimation(&twinkling);
        
      } break;
      
      case 3: {
      
        static Animation twinkling = animations_newAnimation();
        twinkling.duration = 30.0;
        twinkling.function = &animation_twinkle_RED;
    
        animations_addAnimation(&twinkling);
        
      } break;
      
      case 4: {
        
        full_off(_reactor);
        
        static Animation charging = animations_newAnimation();
        charging.duration = 100.0;
        charging.function = &animation_chargingUp;

        animations_addAnimation(&charging);
        
      } break;
      
      case 5: {

        // Shorten this root cycle because it's not super thrilling.
        animation->duration = animation->duration / 2.0;
        animation->endTime = 0.0;
        animation_boilerplate(animation, current_ms);
        
        static Animation rgb = animations_newAnimation();
        rgb.duration = animation->duration / 2.0;
        rgb.function = &animation_rgb;
        
        animations_addAnimation(&rgb);
        
      } break;
    }
  }
}

// This animation is two keyframes, each of which is its own animation.
// First keyframe is purple being filled by green.
// Second keyframe is green being filled by purple.
void animation_reactor_spin_GREEN(Animation *animation, double current_ms) {
  _animation_reactor_spin(GREEN_COLOR, animation, current_ms);
}

void animation_reactor_spin_RED(Animation *animation, double current_ms) {
  _animation_reactor_spin(RED_COLOR, animation, current_ms);
}

void _animation_reactor_spin(uint16_t color_choice, Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  static Animation purple_filling_with_green;
  static Animation green_filling_with_purple;

  // New animation?
  if (percentage_complete == 0.0) {
    purple_filling_with_green = animations_newAnimation();
    green_filling_with_purple = animations_newAnimation();

    purple_filling_with_green.duration = animation->duration / 2;
    green_filling_with_purple.duration = animation->duration / 2;

    // We will start these up when appropriate.
    purple_filling_with_green.isFinished = true;
    green_filling_with_purple.isFinished = true;
    
    void (*b_filling_with_a_fn)(Animation *, double);
    void (*a_filling_with_b_fn)(Animation *, double);
    
    // Pick the proper color sub animations based on the given color choice.
    switch (color_choice) {
      case GREEN_COLOR:
        a_filling_with_b_fn = &animation_reactor_GreenWithPurpleFillingForward;
        b_filling_with_a_fn = &animation_reactor_PurpleWithGreenFillingForward;
        break;
      case RED_COLOR:
        a_filling_with_b_fn = &animation_reactor_RedWithPurpleFillingForward;
        b_filling_with_a_fn = &animation_reactor_PurpleWithRedFillingForward;
        break;
    }

    purple_filling_with_green.function = b_filling_with_a_fn;
    green_filling_with_purple.function = a_filling_with_b_fn;
    
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

// This is the "short circuit" white noise on the reactor strip.
// Also periodically applies to the weapon strip too.
void animation_noise_fullsystem(Animation *animation, double current_ms) {
  _animation_noise(animation, current_ms, false);
}

void animation_noise_reactor(Animation *animation, double current_ms) {
  _animation_noise(animation, current_ms, true);
}

void _animation_noise(Animation *animation, double current_ms, bool reactor_only) {
  double percentage_complete = animation_boilerplate(animation, current_ms);

  // Reactor
  uint8_t leds = (random() % 2) + 2; // 2 or 3 LEDs.
  uint8_t chance = reactor_only ? 15 : 30;
  bool proc = whiteNoise(_reactor, chance, leds);
  animation->userInfo += proc;
  
  // White noise is random. Each time it procs, we increment our userInfo counter.
  // Once a threshold is met, we light up the other strip too.
  double threshold = 1.0;
  
  // BOOM light up everything on the contraption.
  static Animation arduino_animation;
  if (animation->userInfo >= threshold) {
    animation->userInfo = 0.0;
    
    // The proc is actually slightly too fast.
    if (!reactor_only && random() % 3) {
      // Other strip
      whiteNoise(_weapon, 1, 3 + random() % 5);
      whiteNoise(_weapon, 50, 3 + random() % 7);
      whiteNoise(_weapon, 200, 10);
      // Arduino LEDs
      arduino_animation = animations_newAnimation();
      arduino_animation.duration = 50.0;
      arduino_animation.function = &animate_arduino_LEDs;
      animations_addAnimation(&arduino_animation);
    }
  }
  
  // Last run? Reset our user info.
  if (percentage_complete >= 100.0) {
    animation->userInfo = 0.0;
  }
}

void animation_weapon1(Animation *animation, double current_ms) {
  // Loop forever
  animation->isFinished = false;

  uint16_t const pale_blue = 214;

  animate_backgroundColorWithColorFillingForward(_weapon, OFF_COLOR, pale_blue,
                                                 animation, current_ms, 0, 4);
  animate_backgroundColorWithColorFillingForward(_weapon, OFF_COLOR, pale_blue,
                                                animation, current_ms, 9, 5);
}

/*
o . x X x . o o
o o . x X x . o
o o o . x X x .
o o o o . x X x
o o o o o . x X
*/

void animation_chargingUp(Animation *animation, double current_ms) {
  int const primary = GREEN_COLOR;
  int const secondary = TEAL_COLOR;
  int const tertiary = BLUE_COLOR;
  
  double const speed_delta_ms = 50.0;

  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  // Make this animation speed up and slow down over time.
  if (last_run(percentage_complete)) {
    if (animation->userInfo == 0.0) {
      // First time.
      animation->userInfo = speed_delta_ms;
    }
    
    animation->duration += animation->userInfo;
    
    int const seconds = 1000 * 3;
    if (animation->duration > seconds) {
      animation->userInfo = -1 * speed_delta_ms;
    }
    if (animation->duration < speed_delta_ms) {
      animation->userInfo = speed_delta_ms;
    }
  }

  uint8_t const keyframes = _weapon.numPixels() + 4;
  int leading_pixel = percentage_complete / (100.0 / keyframes);
  
  double percent = (percentage_complete / (100.0 / keyframes)) - leading_pixel;
  percent = percent * 100;
  
  double c = map(percent, 0, 100, tertiary, secondary);
  _weapon.setPixelColor(leading_pixel, color(c));
  
  c = map(percent, 0, 100, secondary, primary);
  _weapon.setPixelColor(leading_pixel - 1, color(c));
  
  _weapon.setPixelColor(leading_pixel - 2, color(primary));
  
  c = map(percent, 0, 100, primary, secondary);
  _weapon.setPixelColor(leading_pixel - 3, color(c));
  
  c = map(percent, 0, 100, secondary, tertiary);
  _weapon.setPixelColor(leading_pixel - 4, color(c));
}

void animation_twinkle_RED(Animation *animation, double current_ms) {
  _animation_twinkle(RED_COLOR, 32, 16, animation, current_ms);
  uint8_t leds = 1 + random() % 6;
  noise(_weapon, 55, leds, BLUE_COLOR);
}

void animation_twinkle_TEAL(Animation *animation, double current_ms) {
  _animation_twinkle(TEAL_COLOR, 32, 16, animation, current_ms);
  uint8_t leds = 1 + random() % 10;
  noise(_weapon, 7, leds, RED_COLOR);
}

void _animation_twinkle(uint16_t base_color, uint16_t big_shift, uint16_t little_shift, Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);

  if (percentage_complete == 0.0) {
    for (uint8_t i = 0; i < _reactor.numPixels(); i++) {
      int8_t adjustment = random() % big_shift;
      adjustment = adjustment * (random() % 2 ? 1 : -1);
      if (base_color == RED_COLOR) adjustment = abs(adjustment);
      uint16_t c = base_color + adjustment;
      _reactor.setPixelColor(i, color(c));
    }

    for (uint8_t i = 0; i < _weapon.numPixels(); i++) {
      int8_t adjustment = random() % little_shift;
      adjustment = adjustment * (random() % 2 ? 1 : -1);
      if (base_color == RED_COLOR) adjustment = abs(adjustment);
      uint16_t c = base_color + adjustment;
      _weapon.setPixelColor(i, color(c));
    }
  }
}

void animation_startup(Animation *animation, double current_ms) {
  bool is_finished_before_boilerplate = animation->isFinished;
  
  double percentage_complete = animation_boilerplate(animation, current_ms);

  if (!animation->isFinished) {
    full_off(_reactor);
    full_off(_weapon);
    _reactor.setBrightness(100);
    _reactor.setPixelColor(0, _pixels.Color(127, 0, 0)); // Red
    _reactor.setPixelColor(1, _pixels.Color(0, 127 * 0.5, 0)); // Green
    _reactor.setPixelColor(2, _pixels.Color(0, 0, 127 * 0.5)); // Blue
  }
  // Last run of this animation.
  else if (!is_finished_before_boilerplate) {
    _reactor.setBrightness(REACTOR_LED_DEFAULT_BRIGHTNESS);
  }
}

void animation_rgb(Animation *animation, double current_ms) {
  bool is_finished_before_boilerplate = animation->isFinished;
  
  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  if (first_run(percentage_complete)) {
    _reactor.setBrightness(100);
  }
  
  if (last_run(percentage_complete)) {
    _reactor.setBrightness(REACTOR_LED_DEFAULT_BRIGHTNESS);
  }

  _reactor.setPixelColor(0, _pixels.Color(127, 0, 0)); // Red
  _reactor.setPixelColor(1, _pixels.Color(0, 127 * 0.5, 0)); // Green
  _reactor.setPixelColor(2, _pixels.Color(0, 0, 127 * 0.5)); // Blue
  
  _weapon.setPixelColor(4, fuzzy_color(RED_COLOR, 15));
  _weapon.setPixelColor(3, fuzzy_color(YELLOW_COLOR, 15));
  _weapon.setPixelColor(2, fuzzy_color(GREEN_COLOR, 15));
  _weapon.setPixelColor(1, fuzzy_color(BLUE_COLOR, 15));
  _weapon.setPixelColor(0, fuzzy_color(PURPLE_COLOR, 15));
  
  _weapon.setPixelColor(5, fuzzy_color(RED_COLOR, 15));
  _weapon.setPixelColor(6, fuzzy_color(YELLOW_COLOR, 15));
  _weapon.setPixelColor(7, fuzzy_color(GREEN_COLOR, 15));
  _weapon.setPixelColor(8, fuzzy_color(BLUE_COLOR, 15));
  _weapon.setPixelColor(9, fuzzy_color(PURPLE_COLOR, 15));
}

void animation_weapon_off(Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);
  full_off(_weapon);
}

void animation_reactor_off(Animation *animation, double current_ms) {
  double percentage_complete = animation_boilerplate(animation, current_ms);
  full_off(_reactor);
}

void animation_reactor_PurpleWithGreenFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, PURPLE_COLOR, GREEN_COLOR,
                                                 animation, current_ms);
}

void animation_reactor_GreenWithPurpleFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, GREEN_COLOR, PURPLE_COLOR,
                                                animation, current_ms);
}

void animation_reactor_PurpleWithRedFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, PURPLE_COLOR, RED_COLOR,
                                                 animation, current_ms);
}

void animation_reactor_RedWithPurpleFillingForward(Animation *animation, double current_ms) {
  animate_backgroundColorWithColorFillingForward(_reactor, RED_COLOR, PURPLE_COLOR,
                                                animation, current_ms);
}

// This is an animation where the background is filled with a solid color, and a second color animates in across it.
// First keyframe is all background color.
// Second keyframe is first pixel colored.
// Third keyframe is second pixel colored.
// Etc.
void animate_backgroundColorWithColorFillingForward(Adafruit_NeoPixel pixels, uint16_t back, uint16_t fill,
                                                    Animation *animation, double current_ms, int8_t lowRange, int8_t highRange) {
  if (animation->isFinished) {
    return;
  }

  double percentage_complete = animation_boilerplate(animation, current_ms);
  if (percentage_complete > 100.0) return;

  if (lowRange == -1) {
    lowRange = 0;
  }
  if (highRange == -1) {
    highRange = pixels.numPixels() - 1;
  }

  bool swapDirection = highRange < lowRange;

  // Now push some pixels!
  
  // First keyframe is all background color.
  // Second keyframe is first pixel colored.
  // Third keyframe is second pixel colored.
  // Etc.
  uint8_t pixelCount = max(highRange, lowRange) - min(highRange, lowRange) + 1;

  double const numberOfKeyframes = 1 + pixelCount;
  
  // This animation fills in LEDs from left to right. Choose the furthest right pixel.
  int right_pixel = swapDirection
      ? lowRange - floor((percentage_complete / (100.0 / numberOfKeyframes)))
      : lowRange + floor((percentage_complete / (100.0 / numberOfKeyframes)));
  
  // Fill in everything up to the right.
  if (swapDirection) {
    for (uint8_t i = lowRange; i > right_pixel; i--) {
      pixels.setPixelColor(i, color(fill));
    }
  } else {
    for (int8_t i = lowRange; i < right_pixel; i++) {
      pixels.setPixelColor(i, color(fill));
    }
  }
  
  // Fill in everything else.
  if (swapDirection) {
    for (uint8_t i = right_pixel; i >= highRange; i--) {
      pixels.setPixelColor(i, color(back));
    }
  } else {
    for (uint8_t i = right_pixel; i <= highRange; i++) {
      pixels.setPixelColor(i, color(back));
    }
  }
}

// Turns on the arduino LEDs for the duration of the animation.
void animate_arduino_LEDs(Animation *animation, double current_ms) {
  if (animation->isFinished) {
    return;
  }

  double percentage_complete = animation_boilerplate(animation, current_ms);
  
  if (percentage_complete == 0.0) {
    TXLED1;
  }
  else if (percentage_complete >= 100) {
    TXLED0;
  }
}

// Lights up a specific number of pixels. Chance to occure is 1/chance.
// Returns true if chance occurs.
bool whiteNoise(Adafruit_NeoPixel pixels, uint16_t chance, uint16_t number_to_light) {
  noise(pixels, chance, number_to_light, WHITE_COLOR);
}

bool noise(Adafruit_NeoPixel pixels, uint16_t chance, uint16_t number_to_light, uint16_t pixel_color) {
  if (random() % chance == 0) {
    for (uint16_t i = 0; i < number_to_light; i++) {
      uint16_t pixel = random() % pixels.numPixels();
      pixels.setPixelColor(pixel, color(pixel_color));
    }
    return true;
  }
  return false;
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
  a.endTime = 0.0;
  a.isFinished = false;
  // a.function = animation_function;
  a.zIndex = 1;
  a.userInfo = 0.0;

  return a;
}

// Run this in the beginning of all animations.
// Returns the animation percentage given the current time.
double animation_boilerplate(Animation *animation, double current_ms) {
  double const animation_length_ms = animation->duration;
  double end_time_ms = animation->endTime;

  // Is this the first run?
  if (end_time_ms == 0.0) {
    end_time_ms = current_ms + animation_length_ms;
    animation->endTime = end_time_ms;
  }
  // Expired animation?
  else if (current_ms >= end_time_ms) {
    // Reset this animation for next time.
    animation->endTime = 0.0;
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
  // Ensure this animation is not already stored in the animation list.
  for (uint8_t i = 0; i < animations_count; i++) {
    Animation *x = animations[i];
    if (a == x) {
      // Animation is already stored.
      return;
    }
  }

  animations[++animations_count - 1] = a;
  
  // Don't sort the animations here or we might execute an animation twice.
  // This could have bad side effects eg if the animation is resetting this loop.
  // Instead sort the animations at the beginning of the animation loop.
}

// Sort the animations by z-index, lower index comes first.
void animations_sortAnimations() {
  if (animations_count <= 0) return;
  
  // Bubble sort!
  for (uint8_t x = 0; x < animations_count - 1; x++) {
    for (uint8_t y = 0; y < animations_count - x - 1; y++) {
      if (animations[y]->zIndex > animations[y + 1]->zIndex) {
        Animation *swap = animations[y];
        animations[y] = animations[y + 1];
        animations[y + 1] = swap;
      }
    }
  }
}

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

uint32_t fuzzy_color(int16_t c, uint8_t fuzziness) {
  int16_t fuzz = random() % fuzziness;
  if (random() % 2) fuzz = fuzz * -1;
  uint16_t fuzzed_color = c + fuzz;
  if (fuzzed_color > 383) fuzzed_color = 383;
  return color(fuzzed_color);
}

// Color 0 from 383.
// -1 is white.
// Brightness is a multiplier from 1.0 to 0.0.

uint32_t color(int16_t color, float brightness)  {
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
  r = r * brightness;
  g = g * brightness;
  b = b * brightness;
  return _pixels.Color(r, g, b);
}

uint32_t whiteColor() {
  return _pixels.Color(255, 255, 254);
}
