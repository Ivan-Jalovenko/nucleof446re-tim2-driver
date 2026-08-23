#include "stm32f4xx.h"

#include "TIM_driver.h"

void TIM_ClockEnable() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
}

TIM_ExitCode_t TIM_Start() {
    TIM2->CR1 |= TIM_CR1_CEN;
    return TIM_ExitCode_Ok;
}

TIM_ExitCode_t TIM_Stop() {
    TIM2->CR1 &= ~TIM_CR1_CEN;
    return TIM_ExitCode_Ok;
}

TIM_ExitCode_t TIM_Init(TIM_Config_t conf) {
    if (conf.millies == 0) { return TIM_ExitCode_ZeroMsDelay; }

    TIM_Stop();
    TIM2->PSC = 84000 - 1; // PCS value divides the 84MHz timer to 1kHz so 1 tick = 1 ms
    TIM2->ARR = conf.millies - 1;
    
    /* Generate an update event and clear it for PSC value
     * to be loaded into shadow register */
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->CR1 &= ~TIM_CR1_DIR;

    if (conf.one_shot) {
        TIM2->CR1 |= TIM_CR1_OPM;
    } else {
        TIM2->CR1 &= ~TIM_CR1_OPM;
    }

    if (conf.interrupt_driven) {
        TIM2->DIER |= TIM_DIER_UIE;
        NVIC_EnableIRQ(TIM2_IRQn);
        __enable_irq();
    } else {
        TIM2->DIER &= ~TIM_DIER_UIE;
    }

    return TIM_ExitCode_Ok;
}

TIM_ExitCode_t TIM_Delay(uint32_t millies) {
    if (millies == 0) { return TIM_ExitCode_ZeroMsDelay; }

    /* Set up timer for one use and disable update interrupt */
    TIM_Stop();
    TIM2->PSC = 84000 - 1; // PCS value divides the 84MHz timer to 1kHz so 1 tick = 1 ms
    TIM2->ARR = millies - 1;
    
    /* Generate an update event and clear it for PSC value
     * to be loaded into shadow register */
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;

    TIM2->CR1 &= ~TIM_CR1_DIR;
    TIM2->CR1 |= TIM_CR1_OPM;
    TIM2->DIER &= ~TIM_DIER_UIE;
    TIM_Start();

    while (!(TIM2->SR & TIM_SR_UIF)) {
        /* do nothing */
    }

    TIM2->SR &= ~TIM_SR_UIF;
    return TIM_ExitCode_Ok;
}

void TIM2_IRQHandler(void) {
    TIM2->SR &= ~TIM_SR_UIF;
    (void)TIM2->SR;
    
    TIM_UserHandler();
}
