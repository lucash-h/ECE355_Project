#include "timer.h"

void TIM3_config(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 48000 - 1;
    TIM3->ARR = 100 - 1;
    TIM3->CR1 |= TIM_CR1_CEN;
}

void tim3_delay(void) {
    TIM3->CNT = 0;
    while (TIM3->CNT < 1000);
}