#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H
#include <Arduino.h>
#include <ArduinoJson.h>

#include <http_time_sync.h>


typedef struct {
  //all time is stored in second
  unsigned long start_timer;
  unsigned long delta_timer;
  bool action;
} timer_action;

typedef struct {
  unsigned long start;
  unsigned long stop;
} routine_action;

class PinManager
{
public:
  void init_pin(int r_pin, int b_pin);
  bool set_relay(bool set);
  bool isButtonPressed();
  JsonDocument status();
  void manage_timer(ClockTime curr_time);
  bool create_timer(unsigned long delta_timer, bool action, unsigned long current_time);
  bool create_routine(unsigned long start_hour, unsigned long start_minute, unsigned long stop_hour, unsigned long stop_minute);
  bool delete_routine();

private:
  void manage_routine(unsigned long curr_time);
  bool value = false;
  int prev_button_state = LOW;
  int button_state;

  int relay_pin;
  int button_pin;

  //can be only max 10 timers
  timer_action timers[10];
  int timers_running = -1;

  routine_action routine;
  int routine_running = 0;
};
#endif