# STM32F446RE TIM2 Bare-Metal Timer Driver

A small, register-level (no HAL) timer driver for the STM32F446RE
(NUCLEO-F446RE), built while learning bare-metal STM32 programming from
the reference manual directly.

## What it does

- Blocking millisecond delay (`TIM_Delay`), similar in spirit to HAL's
  `HAL_Delay()`.
- Configurable one-shot or repeating timer, in either polling or
  interrupt-driven mode (`TIM_Init` + `TIM_Start`/`TIM_Stop`).
- Correct handling of TIM2's shadow (buffered) PSC/ARR registers — a
  forced update event (`EGR.UG`) is issued on configuration so the new
  period takes effect immediately instead of after one stale cycle.

## What it deliberately does *not* do

- No HAL/LL peripheral library dependency — only CMSIS core headers
  (`core_cm4.h`, for NVIC access and `__enable_irq()`), since ARM's
  interrupt-control instructions aren't something you can reach without
  either this header or hand-written inline assembly.
- Single timer instance only (TIM2), hardcoded — not written to support
  arbitrary timer peripherals.
- Not reentrant / not RTOS-safe as-is — `TIM_Delay` and interrupt-driven
  mode both take exclusive ownership of TIM2 for their duration. Calling
  one while the other is active will produce incorrect behavior.

## Assumptions

- TIM2's timer input clock is **84 MHz** (`CLOCK_HZ` in `TIM_driver.h`).
  This matches the APB1 timer clock under a typical 84 MHz system clock
  configuration on this board. If your clock configuration differs,
  update `CLOCK_HZ` accordingly — the driver derives its prescaler from
  this value, so an incorrect value here means every timed delay is
  proportionally wrong.

## Usage

```c
#include "TIM_driver.h"

int main(void)
{
    TIM_ClockEnable();

    /* Simple blocking delay */
    TIM_Delay(500);

    /* Repeating, interrupt-driven mode */
    TIM_Config_t cfg = {
        .millies = 1000,
        .one_shot = 0,
        .interrupt_driven = 1,
    };
    TIM_Init(cfg);
    TIM_Start();

    for (;;) {
        /* CPU is free — TIM_UserHandler() below runs on each period */
    }
}

/* Must be implemented by the user for interrupt-driven mode.
 * Called from TIM2_IRQHandler after the update flag has already
 * been cleared — no need to clear it yourself. */
void TIM_UserHandler(void)
{
    /* e.g. toggle an LED, set a flag, etc. */
}
```

## Files

- `TIM_driver.h` — public API, config struct, error codes.
- `TIM_driver.c` — implementation.

## Background

Built as part of learning bare-metal STM32 register-level programming —
no CubeMX-generated init code, every register derived from RM0390 and
PM0214 directly. See commit history for the iteration process (this
went through several rounds of bug-fixing: an off-by-one in the
prescaler calculation, missing shadow-register handling, and a couple
of clock-enable ordering issues along the way).STM32F446RE TIM2 Bare-Metal Timer Driver

A small, register-level (no HAL) timer driver for the STM32F446RE (NUCLEO-F446RE), built while learning bare-metal STM32 programming from the reference manual directly.

What it does
Blocking millisecond delay (TIM_Delay), similar in spirit to HAL's HAL_Delay().
Configurable one-shot or repeating timer, in either polling or interrupt-driven mode (TIM_Init + TIM_Start/TIM_Stop).
Correct handling of TIM2's shadow (buffered) PSC/ARR registers — a forced update event (EGR.UG) is issued on configuration so the new period takes effect immediately instead of after one stale cycle.
What it deliberately does not do
No HAL/LL peripheral library dependency — only CMSIS core headers (core_cm4.h, for NVIC access and __enable_irq()), since ARM's interrupt-control instructions aren't something you can reach without either this header or hand-written inline assembly.
Single timer instance only (TIM2), hardcoded — not written to support arbitrary timer peripherals.
Not reentrant / not RTOS-safe as-is — TIM_Delay and interrupt-driven mode both take exclusive ownership of TIM2 for their duration. Calling one while the other is active will produce incorrect behavior.
Assumptions
TIM2's timer input clock is 84 MHz (CLOCK_HZ in TIM_driver.h). This matches the APB1 timer clock under a typical 84 MHz system clock configuration on this board. If your clock configuration differs, update CLOCK_HZ accordingly — the driver derives its prescaler from this value, so an incorrect value here means every timed delay is proportionally wrong.
Usage
c
#include "TIM_driver.h"

int main(void)
{
    TIM_ClockEnable();

    /* Simple blocking delay */
    TIM_Delay(500);

    /* Repeating, interrupt-driven mode */
    TIM_Config_t cfg = {
        .millies = 1000,
        .one_shot = 0,
        .interrupt_driven = 1,
    };
    TIM_Init(cfg);
    TIM_Start();

    for (;;) {
        /* CPU is free — TIM_UserHandler() below runs on each period */
    }
}

/* Must be implemented by the user for interrupt-driven mode.
 * Called from TIM2_IRQHandler after the update flag has already
 * been cleared — no need to clear it yourself. */
void TIM_UserHandler(void)
{
    /* e.g. toggle an LED, set a flag, etc. */
}
Files
TIM_driver.h — public API, config struct, error codes.
TIM_driver.c — implementation.
