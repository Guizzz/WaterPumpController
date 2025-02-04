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
    resp["active_routine"] = routine_running;
    return resp;
}

void PinManager::manage_timer(ClockTime time)
{   
    unsigned long curr_time = time.get_dailySec();
    
    manage_routine(curr_time);
    
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

void PinManager::manage_routine(unsigned long curr_time)
{
    if(routine_running == 0)
        return;
    
    if(routine.start == curr_time)
    {
        Serial.println("Routine started");
        set_relay(true);
        return;
    }
    
    if(routine.stop != curr_time)
        return;

    Serial.println("Routine ended");
    set_relay(false);
}

bool PinManager::create_timer(unsigned long delta_timer, bool action, unsigned long current_time)
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
    timers[i_available].start_timer = current_time;
    timers[i_available].delta_timer = delta_timer;
    Serial.println("Timer #" + String(i_available) + 
                    " started | start " + String(timers[i_available].start_timer) + 
                    " | delta_timer " + String(timers[i_available].delta_timer));
    return true;
}

bool PinManager::create_routine(unsigned long start_hour, unsigned long start_minute, unsigned long stop_hour, unsigned long stop_minute)
{
    routine.start = (start_hour * 3600) + (start_minute * 60);
    routine.stop = (stop_hour * 3600) + (stop_minute * 60);

    routine_running ++;
    
    Serial.print("Routine created | start: "); Serial.print(routine.start);
    Serial.print(" | stop: "); Serial.println(routine.stop);
    return true;
}

bool PinManager::delete_routine()
{   
    if(routine_running > 0)
        routine_running --;
    return true;
}
