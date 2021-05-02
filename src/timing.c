#include "timer.h"
#include "intr.h"

#define TICK_FREQUENCY 1000

volatile uint64 ticks = 0;

void _timer_handler(int v)
{
    ticks++;
}

static inline void initTiming()
{
    Timer_InitPIT(PIT_HZ / TICK_FREQUENCY);
    Intr_SetMask(PIT_IRQ, TRUE);
    Intr_SetHandler(IRQ_VECTOR(PIT_IRQ), _timer_handler);
}

static inline uint64 now()
{
    return ticks;
}

static inline void wait(uint64 ticks)
{
    uint64 end = now() + ticks;
    while (now() < end)
        ;
}
