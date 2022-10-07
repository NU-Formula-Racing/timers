# Virtual Timers Library
By Chris Uustal - October 2022
## Brief
This library provides basic timer functionality with single-use and repeating timers 
which can also call a fuction when the timer expires. These timers can be clustered together
into groups which automatically iterate through all the timers and execute them as necessary, 
also performing additional processing to determine if any timers are experiencing timing 
violations. 

This is part of Northwestern Formula Racing's (NFR) shared set of C++ firmware libraries. 

## How to Use
Include the library in your code by using: 
`#include "virtualTimer.h"`

This library provides two types: 

`VirtualTimer` and `VirtualTimerGroup`

### Virtual Timers
To create a virtual timer, you need to construct it with 3 argument: 
- How long the timer should expire after in milliseconds (eg. 100ms)
    - This value has to be non-zero or it will return an error
    - Data type is `uint32_t`, and all durations are assumed to be positive
- A function pointer to the function you want to call when it expires (`NULL` if unused)
    - This is exactly the same as the name of your function, just without the `()` parentheses at the end 
    - If the function is `NULL`, the timer will still work, but the function will not be called
- If the timer should be single-use or repeating 
    - If you want single-use, use: `VirtualTimer::Type::kSingleUse`
    - If you want repeating, use: `VirtualTimer::Type::kRepeating`

An example declaration of a timer could look as follows: 

```
void run10ms(){/* DO SOMETHING AT 10ms */}
VirtualTimer t10(10U, run10ms, VirtualTimer::Type::kRepeating);
```

In the above example, I've created some dummy function called `run10ms` which would contain code that does something I want done every 10ms. 
I then create a `VirtualTimer` object called `t10` which I then configure to expire after 10ms, call the function `run10ms`, and be repeating. 

After you construct a timer, you must start it for it to begin timing. 
This can be done with the `Start()` member function. 

In the main loop of your code, you must then tick the timer to update it and execute the associated function. 
This can be done with the `Tick()` member function. 

NOTE: IF YOU DO NOT TICK YOUR TIMER, IT WILL NOT DO ANYTHING 

To keep these libraries hardware-agnostic, you must pass each of these functions the current time of the system in milliseconds as a `uint32_t`. For most users on Arduino-similar platforms, this can be done using `millis()`. If you're on alternative platforms, you may need to pass the value of a core-tick register or something similar after having scaled it to milliseconds. 

Continuing the example from before, we would have the following setup and loop blocks: 
```
void setup()
{
    t10.Start(millis());
}

void loop()
{
    t10.Tick(millis());
}
```
And that's it! That's all you need to do to ensure you code is being executed at a standard time interval. 

If you're interested in using one of these timers just as a timer instead of a function caller, you would just pass in a function of `NULL`. For that use case, there are additional helpful member functions you can call: 
- `GetTimerState`
    - This returns if the current state of the timer is `NotStarted`, `Running`, or `Expired`
- `HasTimerExpired`
    - This removes the step of having to check if the timer has expired yourself and instead just returns a `boolean` reporting if the current state is expired or not 
- `GetElapsedTime`
    - Returns the elapsed time between when the timer has started and now 
    - You must pass this function the current time (eg. `millis()`) and it will return the elapsed time as a `uint32_t`

Repeating timers will never expire. A single-use timer can be restarted by calling the `Start` member funciton on it again. 

### Virtual Timer Groups
A virtual timer group is (just as it sounds) a group of virtual timers. 
These timers can either be existing virtual timers or new timers you make just as part of the group. They can be used interchangeably, except you can't call individual timers that are a part of a group out side of that group (they're all just called automatically at their associated timings as part of the group). 

You start by creating a `VirtualTimerGroup` object, which doesn't need to be passed any values. Eg: 

`VirtualTimerGroup timer_group;`

As mentioned (but not recommended if the timer will only be used as part of the group), you can add an existing timer to a group by calling the `AddTimer` member function of a group with a pointer to the virtual timer you want to add. Eg: 

`timer_group.AddTimer(&t10);`

Or (the recommended approach), you can create a timer directly as part of a group. This is done by using the `AddTimer` member function of the timer group and passing in the duration and function pointer (just as you did for a singular virtual timer). Eg: 

`timer_group.AddTimer(10U, run10ms);`

NOTE: Virtual timer groups are only intended for REPEATING timers. This is why you do not need to pass the repeating or single-use parameter. 
If you pass in a single-use timer, it will become a repeating timer. 

You do not need to start timers when they are in a timer group. If the timer is not already started, it will be automatically started the first time you tick the group. A timer group is ticked the same way a timer is, by passing the current system time (eg. `millis()`) to the tick member function of the group. Eg:

`timer_group.Tick();`

## Timing Violations
While the below behavior is not necessary to use or understand to use the library, it may be helpful depending on your application. In particular, all tick function calls will return a bool indicating if timing violations are occuring. Particularly, a virtual timer returning false indicates that the duration between the last tick of the function and the current tick of the function is greater than 2x the duration of the timer. This means that the timer is not executing the given function at the desired speed and may not behave as intended. For a timer group, the minimum timer duration is stored in memory, and if the tick function for the group takes longer to return than the minimum timer duration, it returns false. Similar to an indivudal timer, this indicates that one or more timers in the timer group will not be executing at their desired frequency due to slowed execution elsewhere in the code. 