struct Animation {
  // Animation function, defined somewhere in the implementation.
  // Accepts a pointer to this animation object and the current arduino time in milliseconds.
  void (*function)(Animation *animation, double current_time_ms);
  // Additional info to hold. Currently only used to hold the animation finish time.
  double userInfo;
  // Length of this animation.
  double animationDuration;
  // Set true if this animation has finished.
  bool isFinished;
  // Used for sorting. Higher z-index is sorted above (after) lower z-index animations.
  uint8_t zIndex;

  // For internal use
  double endTime;
};
typedef struct Animation Animation;
