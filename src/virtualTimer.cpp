/**
 * @file virtualTimer.cpp
 * @author Chris Uustal
 * @brief Defines some of then longer functions declared in the virtualTimer.h file
 * @version 1
 * @date 2022-09-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../include/virtualTimer.h"

/**
 * @brief Default constructor for VirtualTimer object
 *
 */
VirtualTimer::VirtualTimer()
{
    duration = 0U;
    task_func = nullptr;
    type = Type::kUninitialized;
}

/**
 * @brief Construct a new Virtual Timer:: Virtual Timer object
 *
 * @param duration_ms - Duration the timer should fire after
 * @param task_func - Function to be called when timer expires
 * @param timer_type - If the timer should be repeating or single-use
 */
VirtualTimer::VirtualTimer(uint32_t duration_ms, std::function<void(void)> task_func, Type timer_type)
{
    if (duration_ms != 0U)
    {
        duration = duration_ms;
        this->task_func = task_func;
        type = timer_type;
    }
}

/**
 * @brief Construct a new Virtual Timer:: Virtual Timer object
 *
 * @param duration_ms - Duration the timer should fire after
 * @param task_func - Function to be called when timer expires
 * @param timer_type - If the timer should be repeating or single-use
 * @param max_calls - Maximum number of times to call the timer
 */
VirtualTimer::VirtualTimer(uint32_t duration_ms,
                           std::function<void(void)> task_func,
                           Type timer_type,
                           uint16_t max_calls)
{
    if (duration_ms != 0U)
    {
        duration = duration_ms;
        this->task_func = task_func;
        type = timer_type;
        maximum_calls = max_calls;
        call_counter = 0;
    }
}

/**
 * @brief Constructor duplicate for times when we can't use a parametrized constructor
 *
 * @param duration_ms - Duration the timer should fire after
 * @param task_func - Function to be called when timer expires
 * @param timer_type - If the timer should be repeating or single-use
 */
void VirtualTimer::Init(uint32_t duration_ms, std::function<void(void)> task_func, Type timer_type)
{
    if (duration_ms != 0U)
    {
        duration = duration_ms;
        this->task_func = task_func;
        type = timer_type;
    }
}

/**
 * @brief Start the associated timer
 *
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 */
void VirtualTimer::Start(uint32_t current_time)
{
    state = State::kRunning;
    prev_tick = current_time;
}

/**
 * @brief Get the current state of the timer (do not update it)
 *
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return Current state of the timer
 */
VirtualTimer::State VirtualTimer::GetTimerState() { return state; }

/**
 * @brief Check if the timer has expired but do not update it
 *
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return true = timer has exceeded duration
 * @return false = timer has not exceeded duration
 */
bool VirtualTimer::HasTimerExpired()
{
    bool ret = false;
    if (GetTimerState() == State::kExpired)
    {
        ret = true;
    }
    return ret;
}

/**
 * @brief Get the time (in ms) that has elapsed since the timer was started
 *
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return Elapsed time since timer was started
 */
uint32_t VirtualTimer::GetElapsedTime(uint32_t current_time)
{
    uint32_t ret = current_time - prev_tick;
    return ret;
}

/**
 * @brief Run the fuction associated with this timer and update prev_tick if it's a repeating timer
 *
 * @param current_time - the current time in ms at the time of calling (eg. millis())
 * @return true if the function returned faster than the timer duration
 * @return false if the function pointer is null or task missed an iteration
 */
bool VirtualTimer::Tick(uint32_t current_time)
{
    bool ret = true;
    // Only tick a timer if it's already running
    if (state == State::kRunning)
    {
        // Check if timer has expired
        if (current_time >= prev_tick + duration)
        {
            if (type == Type::kRepeating || type == Type::kFiniteUse)
            {
                // Check if the timer missed a cycle
                if (current_time > prev_tick + 2 * duration)
                {
                    ret = false;
                }
                // Update the last tick to the current time
                prev_tick = current_time;
            }
            else
            {
                // Timer isn't repeating, so it expires
                state = State::kExpired;
            }

            if (type == Type::kFiniteUse)
            {
                call_counter += 1;
                // If we reach maximum number of calls, timer expires
                if (call_counter >= maximum_calls)
                {
                    state = State::kExpired;
                }
            }

            if (task_func != nullptr)
            {
                // Run the associated task function
                task_func();
            }
            else
            {
                ret = false;
            }
        }
    }

    return ret;
}

/**
 * @brief Default constructor for VirtualTimerGroup
 *
 */
VirtualTimerGroup::VirtualTimerGroup() {}

/**
 * @brief Add an existing timer to the timer group
 *
 * @param newTimer = existig timer to be added to the group
 */
void VirtualTimerGroup::AddTimer(VirtualTimer &new_timer)
{
    if (timer_group.empty())
    {
        min_timer_duration = new_timer.duration;
    }
    else if (new_timer.duration < min_timer_duration)
    {
        min_timer_duration = new_timer.duration;
    }

    timer_group.push_back(&new_timer);
}

/**
 * @brief Create a new timer with the given duration and function and add it to the group
 *
 * @param duration = duration between function calls
 * @param runFunc = function to be run when ticked
 * @return true = timer was added successfully
 * @return false = failed to add timer (likely reached the max number of timers)
 */
VirtualTimer *VirtualTimerGroup::AddTimer(uint32_t duration_ms, std::function<void(void)> task_func)
{
    if (timer_group.empty() || duration_ms < min_timer_duration)
    {
        min_timer_duration = duration_ms;
    }

    timer_group.emplace_back(new VirtualTimer(duration_ms, task_func, VirtualTimer::Type::kRepeating));
    return timer_group.back();
}

VirtualTimer *VirtualTimerGroup::AddTimer(uint32_t duration_ms, std::function<void(void)> task_func, uint16_t max_calls)
{
    if (timer_group.empty() || duration_ms < min_timer_duration)
    {
        min_timer_duration = duration_ms;
    }

    timer_group.emplace_back(new VirtualTimer(duration_ms, task_func, VirtualTimer::Type::kFiniteUse, max_calls));
    return timer_group.back();
}

/**
 * @brief Tick all timers in group
 *
 * @return true if all tasks ran faster than minimum timer duration
 * @return false if the task loop took longer than the minimum timer duration
 * 			(timers may no longer fire at the correct interval)
 */
bool VirtualTimerGroup::Tick(uint32_t current_time)
{
    bool ret = true;

    if (timer_group.empty() == false)
    {
        // If the time since the last tick is longer than the shortest task
        if ((current_time - prev_tick) > min_timer_duration)
        {
            ret = false;
        }

        std::vector<VirtualTimer *>::iterator i;
        for (i = timer_group.begin(); i != timer_group.end(); i++)
        {
            if ((*i)->GetTimerState() == VirtualTimer::State::kNotStarted)
            {
                (*i)->Start(current_time);
            }
            else if ((*i)->GetTimerState() == VirtualTimer::State::kExpired)
            {
                timer_group.erase(i);
            }

            if ((*i)->Tick(current_time) == false)
            {
                ret = false;
            }
        }
    }

    prev_tick = current_time;

    return ret;
}
