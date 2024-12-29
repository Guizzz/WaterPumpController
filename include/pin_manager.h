#include<Arduino.h>

#define REALY_PIN 5

class PinManager
{
public:
  void init_pin();
  bool set_relay(bool set);
  bool status();
private:
  bool value = false;
};