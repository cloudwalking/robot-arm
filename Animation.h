struct Animation {
  // Animation function, defined somewhere in the implementation.
  // Accepts a pointer to this animation object and the current arduino time in milliseconds.
  void (*function)(Animation *animation, double current_time_ms);
  double userInfo;
  bool isFinished;
};
typedef struct Animation Animation;
