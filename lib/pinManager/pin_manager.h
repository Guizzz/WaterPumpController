#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H
#include<Arduino.h>
#include <ArduinoJson.h>

#define REALY_PIN 5

typedef struct {
  //all time is stored in second
  unsigned long start_timer;
  unsigned long delta_timer;
  bool action;
} timer_action;

class PinManager
{
public:
  void init_pin();
  bool set_relay(bool set);
  JsonDocument status();
  void manage_timer(unsigned long curr_time);
  bool create_timer(unsigned long delta_timer, bool action);
private:
  bool value = false;

  //can be only max 10 timers
  timer_action timers[10];
  int timers_running = -1;
};
#endif