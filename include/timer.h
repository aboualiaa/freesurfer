#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <string>

/// \class Timer
///
/// A simple, high-resolution chronometer.
///
/// The Timer class can be used to track elapsed time with nanosecond
/// resolution. The timer begins at construction but can be reset with the
/// `reset()` function.

class Timer {
public:
  Timer() : begin(clock::now()) {}

  // retrieves elapsed time
  long   nanoseconds();
  long   milliseconds();
  double seconds();
  double minutes();
  double hours();

  // resets the timer
  void reset();

private:
  using clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<clock> begin;
};

#define TIMER_INTERVAL_BEGIN(NAME) Timer NAME;
#define TIMER_INTERVAL_END(NAME)                                               \
  std::cout << __FILE__ << ":" << __LINE__ << " interval took "                \
            << NAME.milliseconds() << " msec" << std::endl;

std::string currentDateTime(bool allowOverride = true);

#endif
