struct Animation {
  // Animation function, defined somewhere in the implementation.
  // Accepts a pointer to this animation object and the current arduino time in milliseconds.
  void (*function)(Animation *animation, double current_time_ms);
  // Additional info to hold. Currently only used to hold the animation finish time.
  double userInfo;
  // Length of this animation.
  double animationLength;
  // Set true if this animation has finished.
  bool isFinished;
};
typedef struct Animation Animation;
