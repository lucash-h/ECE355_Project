//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
// School: University of Victoria, Canada.
// Course: ECE 355 "Microprocessor-Based Systems".
// This is tutorial code for Part 1 of Introductory Lab.
//
// See "system/include/cmsis/stm32f051x8.h" for register/bit definitions.
// See "system/src/cmsis/vectors_stm32f051x8.c" for handler declarations.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"

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
/* Delay count for TIM2 timer: 1/4 sec at 48 MHz */
#define myTIM2_PERIOD ((uint32_t)12000000)


void myGPIOA_Init(void);
void myGPIOC_Init(void);
void myTIM2_Init(void);


/* Global variable indicating which LED is blinking */
volatile uint16_t blinkingLED = ((uint16_t)0x0100);


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


	// By customizing __initialize_args() it is possible to pass arguments,
	// for example when running tests with semihosting you can pass various
	// options to the test.
	// trace_dump_args(argc, argv);

	// Send a greeting to the trace device (skipped on Release).
	trace_puts("Hello World!");

	// The standard output and the standard error should be forwarded to
	// the trace device. For this to work, a redirection in _write.c is
	// required.
	puts("Standard output message.");
	fprintf(stderr, "Standard error message.\n");

	// At this stage the system clock should have already been configured
	// at high speed.
	trace_printf("System clock: %u Hz\n", SystemCoreClock);


	myGPIOA_Init();		/* Initialize I/O port PA */
	myGPIOB_Init();		/* Initialize I/O port PB */
	myGPIOC_Init();		/* Initialize I/O port PC */
	myTIM2_Init();		/* Initialize timer TIM2 */

	while (1)
	{	//right now when the button is pressed the led will switch between blue and green
		//what i want to do is do that aaaand also set the display to show the frequency and period of the signal
		//but I guess I should probably get the display and the user button working first huhuhuh
		/* If button is pressed, switch between blue and green LEDs */
		if((GPIOA->IDR & GPIO_IDR_0) != 0)
		{
			/* Wait for button to be released (PA0 = 0) */
			while((GPIOA->IDR & GPIO_IDR_0) != 0){}

			/* Turn off currently blinking LED */
			GPIOC->BRR = blinkingLED;
			/* Switch blinking LED */
			blinkingLED ^= ((uint16_t)0x0300);
			/* Turn on switched LED */
			GPIOC->BSRR = blinkingLED;

			trace_printf("\nSwitching the blinking LED...\n");
		}
	}

	return 0;

}


void myGPIOA_Init()
{
	/* Enable clock for GPIOA peripheral */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* Configure PA0 as input */
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);
	/* Ensure no pull-up/pull-down for PA0 */
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0);
}

void myGPIOB_INIT() {
	/* Enable clock for GPIOB peripheral */
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	/* configure PB4 as output*/
	GPIOB->MODER |= (GPIO_MODER_MODER4_0);


	/*push-pull mode for pb4*/
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_4);

	/* Ensure high-speed mode for PB4 */
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR4);

	/*no pullup/pulldown from pb4*/
	GPIOB->PUPDR |= ~(GPIO_PUPDR_PUPDR4);


}

void myGPIOC_Init()
{
	/* Enable clock for GPIOC peripheral */
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

	/* Configure PC8 and PC9 as outputs */
	GPIOC->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);
	/* Ensure push-pull mode selected for PC8 and PC9 */
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);
	/* Ensure high-speed mode for PC8 and PC9 */
	GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9);
	/* Ensure no pull-up/pull-down for PC8 and PC9 */
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
}


void myTIM2_Init()
{
	/* Enable clock for TIM2 peripheral */
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Configure TIM2: buffer auto-reload, count up, stop on overflow,
	 * enable update events, interrupt on overflow only */
	TIM2->CR1 = ((uint16_t)0x008C);

	/* Set clock prescaler value */
	TIM2->PSC = myTIM2_PRESCALER;
	/* Set auto-reloaded delay */
	TIM2->ARR = myTIM2_PERIOD;

	/* Update timer registers */
	TIM2->EGR = ((uint16_t)0x0001);

	/* Assign TIM2 interrupt priority = 0 in NVIC */
	NVIC_SetPriority(TIM2_IRQn, 0);
	// Same as: NVIC->IP[3] = ((uint32_t)0x00FFFFFF);

	/* Enable TIM2 interrupts in NVIC */
	NVIC_EnableIRQ(TIM2_IRQn);
	// Same as: NVIC->ISER[0] = ((uint32_t)0x00008000) */

	/* Enable update interrupt generation */
	TIM2->DIER |= TIM_DIER_UIE;
	/* Start counting timer pulses */
	TIM2->CR1 |= TIM_CR1_CEN;
}


/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM2_IRQHandler()
{
	uint16_t LEDstate;

	/* Check if update interrupt flag is indeed set */
	if ((TIM2->SR & TIM_SR_UIF) != 0)
	{
		/* Read current PC output and isolate PC8 and PC9 bits */
		LEDstate = GPIOC->ODR & ((uint16_t)0x0300);
		if (LEDstate == 0)	/* If LED is off, turn it on... */
		{
			/* Set PC8 or PC9 bit */
			GPIOC->BSRR = blinkingLED;
		}
		else			/* ...else (LED is on), turn it off */
		{
			/* Reset PC8 or PC9 bit */
			GPIOC->BRR = blinkingLED;
		}

		TIM2->SR &= ~(TIM_SR_UIF);	/* Clear update interrupt flag */
		TIM2->CR1 |= TIM_CR1_CEN;	/* Restart stopped timer */
	}
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
