//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
// School: University of Victoria, Canada.
// Course: ECE 355 "Microprocessor-Based Systems".
// This is template code for Part 2 of Introductory Lab.
//
// See "system/include/cmsis/stm32f051x8.h" for register/bit definitions.
// See "system/src/cmsis/vectors_stm32f051x8.c" for handler declarations.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"
#include "stm32f051x8.h"

// ----------------------------------------------------------------------------
//
// STM32F0 empty sample (trace via $(trace)).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the $(trace) output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


/* Definitions of registers and their bits are
   given in system/include/cmsis/stm32f051x8.h */


/* Clock prescaler for TIM2 timer: no prescaling */
#define myTIM2_PRESCALER ((uint16_t)0x0000)
/* Maximum possible setting for overflow */
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)

//manually because stm32f051x8.h was being annoying
#define GPIO_IDR_ID0        (0x00000001U)  // Mask for pin 0 (PA0)


void myGPIOA_Init(void);
void myTIM2_Init(void);
void myEXTI_Init(void);
void TIM3_Init(void);
void tim3_delay(void);


// Declare/initialize your global variables here...
// NOTE: You'll need at least one global variable
// (say, timerTriggered = 0 or 1) to indicate
// whether TIM2 has started counting or not.
uint32_t timerTriggered = 0;
uint8_t current_state = 0; //0 for pa1, 1 for pa2


/*** Call this function to boost the STM32F0xx clock to 48 MHz ***/

void SystemClock48MHz( void )
{
//
// Disable the PLL
//
    RCC->CR &= ~(RCC_CR_PLLON);
//
// Wait for the PLL to unlock
//
    while (( RCC->CR & RCC_CR_PLLRDY ) != 0 );
//
// Configure the PLL for 48-MHz system clock
//
    RCC->CFGR = 0x00280000;
//
// Enable the PLL
//
    RCC->CR |= RCC_CR_PLLON;
//
// Wait for the PLL to lock
//
    while (( RCC->CR & RCC_CR_PLLRDY ) != RCC_CR_PLLRDY );
//
// Switch the processor to the PLL clock source
//
    RCC->CFGR = ( RCC->CFGR & (~RCC_CFGR_SW_Msk)) | RCC_CFGR_SW_PLL;
//
// Update the system with the new clock frequency
//
    SystemCoreClockUpdate();

}

/*****************************************************************/


int
main(int argc, char* argv[])
{

	SystemClock48MHz();

	trace_printf("This is the button piece of this lab project\n");
	trace_printf("System clock: %u Hz\n", SystemCoreClock);

	myGPIOA_Init();		/* Initialize I/O port PA */
	myTIM2_Init();		/* Initialize timer TIM2 */
	myEXTI_Init();		/* Initialize EXTI */
	TIM3_Init();

	while (1)
	{
		if(current_state == 0) {
			trace_printf("\nPA2 is OFF\n");
		} else {
			trace_printf("\nPA2 is ON\n");
		}
		//delay loop
		tim3_delay();

	}

	return 0;

}


void myGPIOA_Init()
{
	/* Enable clock for GPIOA peripheral */
	// Relevant register: RCC->AHBENR
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    /* Configure PA0-2 as input mode*/
    //relevant register: GPIOA->MODER
    GPIOA->MODER &= ~(GPIO_MODER_MODER0);
    GPIOA->MODER &= ~(GPIO_MODER_MODER1);
	GPIOA->MODER &= ~(GPIO_MODER_MODER2);


	/* Ensure no pull-up/pull-down(resistors) for PA0-2 */
	// Relevant register: GPIOA->PUPDR

	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR1);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2);
}


void myTIM2_Init()
{
	/* Enable clock for TIM2 peripheral */
	// Relevant register: RCC->APB1ENR
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Configure TIM2: buffer auto-reload, count up, stop on overflow,
	 * enable update events, interrupt on overflow only */
	// Relevant register: TIM2->CR1
	TIM2->CR1 = ((uint16_t)0x008C);

	/* Set clock prescaler value */
	TIM2->PSC = myTIM2_PRESCALER;
	/* Set auto-reloaded delay */
	TIM2->ARR = myTIM2_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM2->EGR
	TIM2->EGR |= ((uint16_t)0x0001);

	/* Assign TIM2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[3], or use NVIC_SetPriority
	NVIC_SetPriority(TIM2_IRQn, 0);

	/* Enable TIM2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(TIM2_IRQn);

	/* Enable update interrupt generation */
	// Relevant register: TIM2->DIER
	TIM2->DIER |= TIM_DIER_UIE;
}

void TIM3_Init(void) {
	//enable tim 3 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	//Set prescaler to get a 1 ms time base through 48 mhz sys clock
	TIM3->PSC = 48000 -1; //prescaler value (48 MHz / 48000 = 1 kHz, so 1 ms per tick)

	// Set auto-reload value for 100 ms (100 ticks at 1 ms per tick)
	TIM3 -> ARR = 100 - 1; //100 ticks each 1 ms

	TIM3->CR1 |= TIM_CR1_CEN;
}

//tim3 is configured specifically for 100 ms

void tim3_delay(void) {
	// Reset the counter and clear the UIF flag
	TIM3->CNT = 0;
	TIM3->SR &= ~TIM_SR_UIF;  // Clear the UIF flag

	// Wait for UIF flag to be set (indicating the timer overflowed)
	while ((TIM3->SR & TIM_SR_UIF) == 0);  // Poll for UIF flag

	// Clear the UIF flag
	TIM3->SR &= ~TIM_SR_UIF;
}


void myEXTI_Init()
{
    /* Map EXTI0 line to PA0 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA;

	/* EXTI0 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	EXTI->RTSR |= EXTI_RTSR_TR0;

	/* Unmask interrupts from EXTI0 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR |= EXTI_IMR_MR0;

	/* Assign EXTI0 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[2], or use NVIC_SetPriority
	NVIC_SetPriority(EXTI0_1_IRQn,0);

	/* Enable EXTI0 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(EXTI0_1_IRQn);

	/* Map EXTI2 line to PA2 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI2_PA;

	/* EXTI2 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	EXTI->RTSR |= EXTI_RTSR_TR2;

	/* Unmask interrupts from EXTI2 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR |= EXTI_IMR_MR2;

	/* Assign EXTI2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[2], or use NVIC_SetPriority
	NVIC_SetPriority(EXTI2_3_IRQn,0);

	/* Enable EXTI2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(EXTI2_3_IRQn);
}


//FOR TESTING JUST THE BUTTON, USING A SWITCH IN BETWEEN NOTHING AND PA2(FUNCTION GENERATOR FROM FIRST PART)
//For final test use function gen and ne555
void EXTI0_1_IRQHandler() {
    if((EXTI->PR & EXTI_PR_PR0) != 0) { //if pending register is set
        EXTI->PR |= EXTI_PR_PR0; //clear pending register

        trace_printf("\nBUTTON PRESSED\n");
        tim3_delay();

        //uint32_t debounce_count = 0;
        while((GPIOA->IDR & GPIO_IDR_ID0) != 0){
            tim3_delay();

        } // wait for the button to be released

        trace_printf("\nBUTTON RELEASED\n");
        if(current_state == 0) {
        	trace_printf("CURRENT STATE WAS 0, NOW 1");
        	current_state = 1;//switch state var to other
        } else {
        	trace_printf("CURRENT STATE WAS 1, NOW 0");
        	current_state = 0;
        }

    }
}



/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM2_IRQHandler()
{
	/* Check if update interrupt flag is indeed set */
	if ((TIM2->SR & TIM_SR_UIF) != 0)
	{
		trace_printf("\n*** Overflow! ***\n");

		/* Clear update interrupt flag */
		// Relevant register: TIM2->SR
		TIM2->SR &= ~(TIM_SR_UIF);

		/* Restart stopped timer */
		// Relevant register: TIM2->CR1
		TIM2->CR1 |= TIM_CR1_CEN;
	}
}


/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void EXTI2_3_IRQHandler()
{
	// Declare/initialize your local variables here...
	uint32_t clockCycles = 0;
	float period = 0;
	float frequency = 0;

	/* Check if EXTI2 interrupt pending flag is indeed set */
	if ((EXTI->PR & EXTI_PR_PR2) != 0)
	{
		//
		// 1. If this is the first edge:
		//	- Clear count register (TIM2->CNT).
		//	- Start timer (TIM2->CR1).
		if (timerTriggered == 0) {
			timerTriggered = 1;
			TIM2->CNT = ((uint16_t)0x0000);
			TIM2->CR1 |= TIM_CR1_CEN;
		}
		//    Else (this is the second edge):
		//	- Stop timer (TIM2->CR1).
		//	- Read out count register (TIM2->CNT).
		//	- Calculate signal period and frequency.
		else {
			timerTriggered = 0;
			TIM2->CR1 &= ~(TIM_CR1_CEN);
			clockCycles = TIM2->CNT;

			period = (float)clockCycles/(float)SystemCoreClock;
			frequency = 1/period;
			timerTriggered = 1;

			trace_printf("Period of the input signal: %f\n", (uint32_t)period);
			trace_printf("Frequency of the input signal: %f\n", (uint32_t)frequency);
		}
		//	- Print calculated values to the console.
		//	  NOTE: Function trace_printf does not work
		//	  with floating-point numbers: you must use
		//	  "unsigned int" type to print your signal
		//	  period and frequency.
		//
		// 2. Clear EXTI2 interrupt pending flag (EXTI->PR).
		// NOTE: A pending register (PR) bit is cleared
		// by writing 1 to it.
		EXTI->PR |= EXTI_PR_PR2;

	}
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
