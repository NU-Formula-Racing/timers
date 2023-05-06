#include <unity.h>

#include "virtualTimer.cpp"
#include "virtualTimer.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

bool dummyFunction(void) { return true; }

void VirtualRepeatingTimerTest(void)
{
    VirtualTimer t100(100, dummyFunction, VirtualTimer::Type::kRepeating);
    t100.Start(1);
    TEST_ASSERT_TRUE(t100.Tick(10));
    TEST_ASSERT_TRUE(t100.Tick(101));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(200));
    TEST_ASSERT_FALSE(t100.Tick(401));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
}

void VirtualSingleTimerTest(void)
{
    VirtualTimer t100(100, dummyFunction, VirtualTimer::Type::kSingleUse);
    t100.Start(1);
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(10));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(101));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(200));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, t100.GetTimerState());
}

void VirtualFiniteTimerTest(void)
{
    VirtualTimer t100(100, dummyFunction, VirtualTimer::Type::kFiniteUse, 2);
    t100.Start(1);
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(10));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(101));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, t100.GetTimerState());
    TEST_ASSERT_TRUE(t100.Tick(201));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, t100.GetTimerState());
}

void VirtualTimerGroupTest(void)
{
    VirtualTimer ts100(100, dummyFunction, VirtualTimer::Type::kSingleUse);
    VirtualTimer tr100(100, dummyFunction, VirtualTimer::Type::kRepeating);
    VirtualTimer tf100(100, dummyFunction, VirtualTimer::Type::kFiniteUse, 2);

    VirtualTimerGroup timer_group;

    timer_group.AddTimer(ts100);
    timer_group.AddTimer(tr100);
    timer_group.AddTimer(tf100);

    TEST_ASSERT_TRUE(timer_group.Tick(0));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, ts100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tr100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tf100.GetTimerState());

    TEST_ASSERT_TRUE(timer_group.Tick(10));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, ts100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tr100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tf100.GetTimerState());

    TEST_ASSERT_TRUE(timer_group.Tick(101));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, ts100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tr100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tf100.GetTimerState());

    TEST_ASSERT_TRUE(timer_group.Tick(201));
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, ts100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kRunning, tr100.GetTimerState());
    TEST_ASSERT_EQUAL(VirtualTimer::State::kExpired, tf100.GetTimerState());
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(VirtualRepeatingTimerTest);
    RUN_TEST(VirtualSingleTimerTest);
    RUN_TEST(VirtualFiniteTimerTest);
    RUN_TEST(VirtualTimerGroupTest);
    UNITY_END();
}
