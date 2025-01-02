#include "pin_manager.h"

void PinManager::init_pin()
{
    pinMode(REALY_PIN, OUTPUT);
    digitalWrite(REALY_PIN, LOW);
}

bool PinManager::set_relay(bool set)
{
    if(set == value)
        return value;
        
    if(set)
        digitalWrite(REALY_PIN, HIGH);
    else
        digitalWrite(REALY_PIN, LOW); 

    PinManager::value = set?HIGH:LOW;
    return value;
}

bool PinManager::status()
{
    return PinManager::value == HIGH;
}
