#include "system_clock.h"
#include "gpio.h"
#include "timer.h"
#include "oled_display.h"
#include "diag/Trace.h"

volatile unsigned int current_state = 0;

void EXTI0_1_IRQHandler(void) {
    if ((EXTI->PR & EXTI_PR_PR0) != 0) {
        EXTI->PR |= EXTI_PR_PR0;

        if (current_state == 0) {
            current_state = 1;
            trace_printf("Switching to state 1: Frequency measurement\n");
        } else {
            current_state = 0;
            trace_printf("Switching to state 0: Frequency measurement off\n");
        }
    }
}

void EXTI2_3_IRQHandler(void) {
    if ((EXTI->PR & EXTI_PR_PR2) != 0) {
        EXTI->PR |= EXTI_PR_PR2;

        if (current_state == 1) {
            Freq = calculate_frequency();
        }
    }
}

unsigned int calculate_frequency(void) {
    return 1000;
}

int main(int argc, char* argv[]) {
    SystemClock48MHz();
    myGPIO_init();
    TIM3_config();
    oled_config();

    while (1) {
        trace_printf("\nFREQUENCY IS: %d\n", Freq);
        trace_printf("\nRESISTANCE IS: %d\n", Res);

        refresh_OLED();
        tim3_delay();
    }
}