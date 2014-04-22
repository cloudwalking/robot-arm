struct Animation {
  // Animation function, defined somewhere in the implementation.
  // Accepts a pointer to this animation object and the current arduino time in milliseconds.
  bool (*function)(Animation *animation, double current_time_ms);
  double userInfo;
};
typedef struct Animation Animation;
