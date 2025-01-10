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

JsonDocument PinManager::status()
{   
    JsonDocument resp;
    resp["relay_status"] = value == HIGH;
    resp["active_timer"] = timers_running+1;
    return resp;
}

void PinManager::manage_timer(unsigned long curr_time)
{   
    if(timers_running == -1)
        return;

    for(int i=0; i < 10; i++)
    {
        if(timers[i].delta_timer == 0)
            continue;

        if (curr_time - timers[i].start_timer < timers[i].delta_timer)
            continue;

        set_relay(timers[i].action);
        timers[i].delta_timer = 0;
        timers_running--;
        Serial.println("Timer #" + String(i) + " is expired");
    }
}

bool PinManager::create_timer(unsigned long delta_timer, bool action)
{
    if(timers_running == 9)
        return false;

    int i_available;

    for(int i=0; i < 10; i++)
    {
        if (timers[i].delta_timer != 0)
            continue;
        i_available = i;
    }

    timers_running++;
    timers[i_available].action = action;
    timers[i_available].start_timer = millis()/1000;
    timers[i_available].delta_timer = delta_timer;
    Serial.println("Timer #" + String(i_available) + 
                    " started | start " + String(timers[i_available].start_timer) + 
                    " | delta_timer " + String(timers[i_available].delta_timer));
    return true;
}

