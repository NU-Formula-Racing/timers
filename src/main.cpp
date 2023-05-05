#ifdef ARDUINO
#include <Arduino.h>

#include "virtualTimer.h"

void run10ms()
{ /* DO SOMETHING AT 10ms */
}
void run100ms()
{ /* DO SOMETHING AT 100ms */
}
void run250ms() { Serial.println("250ms"); }
void run1000ms()
{ /* DO SOMETHING AT 1000ms */
    Serial.println("1000ms");
}

void run2000ms() { Serial.println("Single Use"); }

VirtualTimer t1000(1000U, run1000ms, VirtualTimer::Type::kRepeating);
VirtualTimer t2000(2000, run2000ms, VirtualTimer::Type::kSingleUse);
VirtualTimerGroup timer_group;

void setup()
{
    Serial.begin(115200);

    // Make your own timers and add them to the group
    timer_group.AddTimer(t1000);
    timer_group.AddTimer(t2000);
    // Or just add them directly as part of the group
    // timer_group.AddTimer(10U, run10ms);
    // timer_group.AddTimer(250U, run250ms);
    timer_group.AddTimer(250, run250ms, 5);
}

void loop()
{
    if (timer_group.Tick(millis()) == false)
    {
        // The time between one tick and the next exceeded the timer for the shortest task
        Serial.println("Timing Violation");
    }
}
#endif