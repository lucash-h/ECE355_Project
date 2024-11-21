//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include <string.h>

#include "cmsis/cmsis_device.h"

// ----------------------------------------------------------------------------
//
// STM32F0 led blink sample (trace via $(trace)).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// To demonstrate POSIX retargetting, reroute the STDOUT and STDERR to the
// trace device and display messages on both of them.
//
// Then demonstrates how to blink a led with 1Hz, using a
// continuous loop and SysTick delays.
//
// On DEBUG, the uptime in seconds is also displayed on the trace device.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the $(trace) output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
// The external clock frequency is specified as a preprocessor definition
// passed to the compiler via a command line option (see the 'C/C++ General' ->
// 'Paths and Symbols' -> the 'Symbols' tab, if you want to change it).
// The value selected during project creation was HSE_VALUE=48000000.
//
/// Note: The default clock settings take the user defined HSE_VALUE and try
// to reach the maximum possible system clock. For the default 8MHz input
// the result is guaranteed, but for other values it might not be possible,
// so please adjust the PLL settings in system/src/cmsis/system_stm32f0xx.c
//
//FOR STM32f051x8 microcontroller with ne555 timer(pa1), function generator, pa2, octocoupler, SSD1306 display

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


/*** This is partial code for accessing LED Display via SPI interface. ***/
#include "stm32f051x8.h"
//...


unsigned int Freq = 0;  // Example: measured frequency value (global variable)
unsigned int Res = 0;   // Example: measured resistance value (global variable)


void oled_Write(unsigned char);
void oled_Write_Cmd(unsigned char);
void oled_Write_Data(unsigned char);
void oled_config(void);
void myGPIO_init(void);

void refresh_OLED(void);




void myGPIOA_Init(void);
void myTIM2_Init(void);
void myEXTI_Init(void);
void TIM3_Init(void);
void tim3_delay(void);

void EXTI2_3_IRQHandler(void);

/* Clock prescaler for TIM2 timer: no prescaling */
#define myTIM2_PRESCALER ((uint16_t)0x0000)
/* Maximum possible setting for overflow */
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)

//manually because stm32f051x8.h was being annoying
#define GPIO_IDR_ID0        (0x00000001U)  // Mask for pin 0 (PA0)

SPI_HandleTypeDef SPI_Handle;

/* Maximum possible setting for overflow */
//Taylor
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)

void myGPIOA_Init(void);
//void myTIM2_Init_Taylor(void);
//void myEXTI_Init(void);
void myADC_Init(void);
void myDAC_Init(void);
float getADCValue(void);
float convertADCVoltageToResistance(uint16_t reading);
uint16_t getDACValue(uint16_t ADCValue);

uint32_t timerTriggered = 0;
float period = 0;
float volt = 0;
uint16_t test = 0;
uint16_t nut = 0;

//
// LED Display initialization commands
//
unsigned char oled_init_cmds[] =
{
    0xAE,
    0x20, 0x00,
    0x40,
    0xA0 | 0x01,
    0xA8, 0x40 - 1,
    0xC0 | 0x08,
    0xD3, 0x00,
    0xDA, 0x32,
    0xD5, 0x80,
    0xD9, 0x22,
    0xDB, 0x30,
    0x81, 0xFF,
    0xA4,
    0xA6,
    0xAD, 0x30,
    0x8D, 0x10,
    0xAE | 0x01,
    0xC0,
    0xA0
};


//
// Character specifications for LED Display (1 row = 8 bytes = 1 ASCII character)
// Example: to display '4', retrieve 8 data bytes stored in Characters[52][X] row
//          (where X = 0, 1, ..., 7) and send them one by one to LED Display.
// Row number = character ASCII code (e.g., ASCII code of '4' is 0x34 = 52)
//
unsigned char Characters[][8] = {
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b01011111, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // !
    {0b00000000, 0b00000111, 0b00000000, 0b00000111, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // "
    {0b00010100, 0b01111111, 0b00010100, 0b01111111, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // #
    {0b00100100, 0b00101010, 0b01111111, 0b00101010, 0b00010010,0b00000000, 0b00000000, 0b00000000},  // $
    {0b00100011, 0b00010011, 0b00001000, 0b01100100, 0b01100010,0b00000000, 0b00000000, 0b00000000},  // %
    {0b00110110, 0b01001001, 0b01010101, 0b00100010, 0b01010000,0b00000000, 0b00000000, 0b00000000},  // &
    {0b00000000, 0b00000101, 0b00000011, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // '
    {0b00000000, 0b00011100, 0b00100010, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // (
    {0b00000000, 0b01000001, 0b00100010, 0b00011100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // )
    {0b00010100, 0b00001000, 0b00111110, 0b00001000, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // *
    {0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // +
    {0b00000000, 0b01010000, 0b00110000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // ,
    {0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // -
    {0b00000000, 0b01100000, 0b01100000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // .
    {0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010,0b00000000, 0b00000000, 0b00000000},  // /
    {0b00111110, 0b01010001, 0b01001001, 0b01000101, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // 0
    {0b00000000, 0b01000010, 0b01111111, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // 1
    {0b01000010, 0b01100001, 0b01010001, 0b01001001, 0b01000110,0b00000000, 0b00000000, 0b00000000},  // 2
    {0b00100001, 0b01000001, 0b01000101, 0b01001011, 0b00110001,0b00000000, 0b00000000, 0b00000000},  // 3
    {0b00011000, 0b00010100, 0b00010010, 0b01111111, 0b00010000,0b00000000, 0b00000000, 0b00000000},  // 4
    {0b00100111, 0b01000101, 0b01000101, 0b01000101, 0b00111001,0b00000000, 0b00000000, 0b00000000},  // 5
    {0b00111100, 0b01001010, 0b01001001, 0b01001001, 0b00110000,0b00000000, 0b00000000, 0b00000000},  // 6
    {0b00000011, 0b00000001, 0b01110001, 0b00001001, 0b00000111,0b00000000, 0b00000000, 0b00000000},  // 7
    {0b00110110, 0b01001001, 0b01001001, 0b01001001, 0b00110110,0b00000000, 0b00000000, 0b00000000},  // 8
    {0b00000110, 0b01001001, 0b01001001, 0b00101001, 0b00011110,0b00000000, 0b00000000, 0b00000000},  // 9
    {0b00000000, 0b00110110, 0b00110110, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // :
    {0b00000000, 0b01010110, 0b00110110, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // ;
    {0b00001000, 0b00010100, 0b00100010, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // <
    {0b00010100, 0b00010100, 0b00010100, 0b00010100, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // =
    {0b00000000, 0b01000001, 0b00100010, 0b00010100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // >
    {0b00000010, 0b00000001, 0b01010001, 0b00001001, 0b00000110,0b00000000, 0b00000000, 0b00000000},  // ?
    {0b00110010, 0b01001001, 0b01111001, 0b01000001, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // @
    {0b01111110, 0b00010001, 0b00010001, 0b00010001, 0b01111110,0b00000000, 0b00000000, 0b00000000},  // A
    {0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b00110110,0b00000000, 0b00000000, 0b00000000},  // B
    {0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00100010,0b00000000, 0b00000000, 0b00000000},  // C
    {0b01111111, 0b01000001, 0b01000001, 0b00100010, 0b00011100,0b00000000, 0b00000000, 0b00000000},  // D
    {0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b01000001,0b00000000, 0b00000000, 0b00000000},  // E
    {0b01111111, 0b00001001, 0b00001001, 0b00001001, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // F
    {0b00111110, 0b01000001, 0b01001001, 0b01001001, 0b01111010,0b00000000, 0b00000000, 0b00000000},  // G
    {0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // H
    {0b01000000, 0b01000001, 0b01111111, 0b01000001, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // I
    {0b00100000, 0b01000000, 0b01000001, 0b00111111, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // J
    {0b01111111, 0b00001000, 0b00010100, 0b00100010, 0b01000001,0b00000000, 0b00000000, 0b00000000},  // K
    {0b01111111, 0b01000000, 0b01000000, 0b01000000, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // L
    {0b01111111, 0b00000010, 0b00001100, 0b00000010, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // M
    {0b01111111, 0b00000100, 0b00001000, 0b00010000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // N
    {0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // O
    {0b01111111, 0b00001001, 0b00001001, 0b00001001, 0b00000110,0b00000000, 0b00000000, 0b00000000},  // P
    {0b00111110, 0b01000001, 0b01010001, 0b00100001, 0b01011110,0b00000000, 0b00000000, 0b00000000},  // Q
    {0b01111111, 0b00001001, 0b00011001, 0b00101001, 0b01000110,0b00000000, 0b00000000, 0b00000000},  // R
    {0b01000110, 0b01001001, 0b01001001, 0b01001001, 0b00110001,0b00000000, 0b00000000, 0b00000000},  // S
    {0b00000001, 0b00000001, 0b01111111, 0b00000001, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // T
    {0b00111111, 0b01000000, 0b01000000, 0b01000000, 0b00111111,0b00000000, 0b00000000, 0b00000000},  // U
    {0b00011111, 0b00100000, 0b01000000, 0b00100000, 0b00011111,0b00000000, 0b00000000, 0b00000000},  // V
    {0b00111111, 0b01000000, 0b00111000, 0b01000000, 0b00111111,0b00000000, 0b00000000, 0b00000000},  // W
    {0b01100011, 0b00010100, 0b00001000, 0b00010100, 0b01100011,0b00000000, 0b00000000, 0b00000000},  // X
    {0b00000111, 0b00001000, 0b01110000, 0b00001000, 0b00000111,0b00000000, 0b00000000, 0b00000000},  // Y
    {0b01100001, 0b01010001, 0b01001001, 0b01000101, 0b01000011,0b00000000, 0b00000000, 0b00000000},  // Z
    {0b01111111, 0b01000001, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // [
    {0b00010101, 0b00010110, 0b01111100, 0b00010110, 0b00010101,0b00000000, 0b00000000, 0b00000000},  // back slash
    {0b00000000, 0b00000000, 0b00000000, 0b01000001, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // ]
    {0b00000100, 0b00000010, 0b00000001, 0b00000010, 0b00000100,0b00000000, 0b00000000, 0b00000000},  // ^
    {0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // _
    {0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // `
    {0b00100000, 0b01010100, 0b01010100, 0b01010100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // a
    {0b01111111, 0b01001000, 0b01000100, 0b01000100, 0b00111000,0b00000000, 0b00000000, 0b00000000},  // b
    {0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // c
    {0b00111000, 0b01000100, 0b01000100, 0b01001000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // d
    {0b00111000, 0b01010100, 0b01010100, 0b01010100, 0b00011000,0b00000000, 0b00000000, 0b00000000},  // e
    {0b00001000, 0b01111110, 0b00001001, 0b00000001, 0b00000010,0b00000000, 0b00000000, 0b00000000},  // f
    {0b00001100, 0b01010010, 0b01010010, 0b01010010, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // g
    {0b01111111, 0b00001000, 0b00000100, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // h
    {0b00000000, 0b01000100, 0b01111101, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // i
    {0b00100000, 0b01000000, 0b01000100, 0b00111101, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // j
    {0b01111111, 0b00010000, 0b00101000, 0b01000100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // k
    {0b00000000, 0b01000001, 0b01111111, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // l
    {0b01111100, 0b00000100, 0b00011000, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // m
    {0b01111100, 0b00001000, 0b00000100, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // n
    {0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b00111000,0b00000000, 0b00000000, 0b00000000},  // o
    {0b01111100, 0b00010100, 0b00010100, 0b00010100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // p
    {0b00001000, 0b00010100, 0b00010100, 0b00011000, 0b01111100,0b00000000, 0b00000000, 0b00000000},  // q
    {0b01111100, 0b00001000, 0b00000100, 0b00000100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // r
    {0b01001000, 0b01010100, 0b01010100, 0b01010100, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // s
    {0b00000100, 0b00111111, 0b01000100, 0b01000000, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // t
    {0b00111100, 0b01000000, 0b01000000, 0b00100000, 0b01111100,0b00000000, 0b00000000, 0b00000000},  // u
    {0b00011100, 0b00100000, 0b01000000, 0b00100000, 0b00011100,0b00000000, 0b00000000, 0b00000000},  // v
    {0b00111100, 0b01000000, 0b00111000, 0b01000000, 0b00111100,0b00000000, 0b00000000, 0b00000000},  // w
    {0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100,0b00000000, 0b00000000, 0b00000000},  // x
    {0b00001100, 0b01010000, 0b01010000, 0b01010000, 0b00111100,0b00000000, 0b00000000, 0b00000000},  // y
    {0b01000100, 0b01100100, 0b01010100, 0b01001100, 0b01000100,0b00000000, 0b00000000, 0b00000000},  // z
    {0b00000000, 0b00001000, 0b00110110, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // {
    {0b00000000, 0b00000000, 0b01111111, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // |
    {0b00000000, 0b01000001, 0b00110110, 0b00001000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // }
    {0b00001000, 0b00001000, 0b00101010, 0b00011100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // ~
    {0b00001000, 0b00011100, 0b00101010, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000}   // <-
};




// Declare/initialize your global variables here...
// NOTE: You'll need at least one global variable
// (say, timerTriggered = 0 or 1) to indicate
// whether TIM2 has started counting or not.
//uint32_t timerTriggered = 0;
uint8_t current_state = 0; //0 for pa1, 1 for pa2







int
main(int argc, char* argv[])
{//uint8_t current_state = 0; //0 for pa1, 1 for pa2


	SystemClock48MHz();
	myGPIO_init();
	myGPIOA_Init();
	myEXTI_Init();
	TIM3_Init();
	oled_config();



	//TAYLOR FUNCTIONS
	//myGPIOA_Init();		/* Initialize I/O port PA */
	//myTIM2_Init();		/* Initialize timer TIM2 */
	//myEXTI_Init();		/* Initialize EXTI */
	myADC_Init();		/* Initialize ADC */
	myDAC_Init();		/* Initialize DAC */


	while (1)
	{
        //...
        trace_printf("\nFREQUENCY IS: %d\n", Freq);
        trace_printf("\nRESISTANCE IS: %d\n", Res);
        if(current_state == 1) { //current state => 1 => PA2
        	//Freq++;
        	trace_printf("\nTRANSFER FROM PA1 TO PA2\n");
        } else {
        	//Res++;
        	trace_printf("\nTRANSFER FROM PA2 TO PA1\n");
        }
		//delay loop
		tim3_delay();

		volt = getADCValue();
		Res = convertADCVoltageToResistance(volt);
//		trace_printf("%u\n", DAC->DHR12R1);
		uint16_t test = getDACValue(Res);
		trace_printf("%u\n", test);

//		volt = DAC->DOR1;
//		float test = 5000*((4095-((float)volt))/4095);
//		trace_printf("%u\n", (uint16_t)test);
		// Nothing is going on here...
//		uint16_t test = convertADCVoltageToResistance(adcValue);
//		trace_printf("%u\n", test);

//		DAC->DHR12R1 = getDACValue(test);
//		trace_printf("%u\n", DAC->DHR12R1);


		refresh_OLED();

		// Reset the counter and clear the UIF flag
		tim3_delay();
		trace_printf("\nFREQUENCY IS: %d\n", Freq);
		trace_printf("\nRESISTANCE IS: %d\n", Res);

	}
}



//
// LED Display Functions
//


void refresh_OLED( void )
{
    // Buffer size = at most 16 characters per PAGE + terminating '\0'
    unsigned char Buffer[17];

    snprintf( Buffer, sizeof( Buffer ), "R: %5u Ohms", Res );
    /* Buffer now contains your character ASCII codes for LED Display
       - select PAGE (LED Display line) and set starting SEG (column)
       - for each c = ASCII code = Buffer[0], Buffer[1], ...,
           send 8 bytes in Characters[c][0-7] to LED Display
    */

    //page & segment addresses for resistance
       oled_Write_Cmd( 0xB2 ); // Set Page Address (0xB and then page num 2)
       oled_Write_Cmd( 0x03 ); // bottom half of segment 0011
       oled_Write_Cmd( 0x10 ); // upper half of segment 0000

       for(unsigned int i = 0; i < sizeof(Buffer) && Buffer[i] != '\0'; i++) {
           unsigned char c = Buffer[i];
           for(int j = 0; j < 8; j++) {
               oled_Write_Data(Characters[c][j]); //grabs the character & relevant bytes from the array
           }
       }



    snprintf( Buffer, sizeof( Buffer ), "F: %5u Hz", Freq );
    /* Buffer now contains your character ASCII codes for LED Display
       - select PAGE (LED Display line) and set starting SEG (column)
       - for each c = ASCII code = Buffer[0], Buffer[1], ...,
           send 8 bytes in Characters[c][0-7] to LED Display
    */

	oled_Write_Cmd( 0xB3 ); // Set Page Address (0xB and then page num 3)
	oled_Write_Cmd( 0x03 ); // bottom half of segment 0011
	oled_Write_Cmd( 0x10 ); // upper half of segment 0000

	for(unsigned int i = 0; i < sizeof(Buffer) && Buffer[i] != '\0'; i++) {
		unsigned char c = Buffer[i];
		trace_printf("\nChar in buffer is *%u* at pos %*d*\n", c, i);
		for(int j = 0; j < 8; j++) {
			oled_Write_Data(Characters[c][j]); //grabs the character & relevant bytes from the array
		}
	}



	/* Wait for ~100 ms (for example) to get ~10 frames/sec refresh rate
       - You should use TIM3 to implement this delay (e.g., via polling)
    */

	tim3_delay();

}
/*
void myGPIOA_Init()
{
	/* Enable clock for GPIOA peripheral */
	// Relevant register: RCC->AHBENR
/*RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* Configure PA2 as input */
	// Relevant register: GPIOA->MODER
/*GPIOA->MODER &= ~(GPIO_MODER_MODER2);


	/* Ensure no pull-up/pull-down for PA2 */
	// Relevant register: GPIOA->PUPDR
/*GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR2);
}

/*
void myTIM2_Init()
{
	/* Enable clock for TIM2 peripheral */
	// Relevant register: RCC->APB1ENR
	/*RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Configure TIM2: buffer auto-reload, count up, stop on overflow,
	 * enable update events, interrupt on overflow only */
	// Relevant register: TIM2->CR1
/*TIM2->CR1 = ((uint16_t)0x008C);

	/* Set clock prescaler value */
/*TIM2->PSC = myTIM2_PRESCALER;
	/* Set auto-reloaded delay */
/*TIM2->ARR = myTIM2_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM2->EGR
/*TIM2->EGR |= ((uint16_t)0x0001);

	/* Assign TIM2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[3], or use NVIC_SetPriority
/*NVIC_SetPriority(TIM2_IRQn, 0);

	/* Enable TIM2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
/*NVIC_EnableIRQ(TIM2_IRQn);

	/* Enable update interrupt generation */
	// Relevant register: TIM2->DIER
/*TIM2->DIER |= TIM_DIER_UIE;
}

/*
void myEXTI_Init()
{
	/* Map EXTI2 line to PA2 */
	// Relevant register: SYSCFG->EXTICR[0]
/*SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI2_PA;

	/* EXTI2 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	/*EXTI->RTSR |= EXTI_RTSR_TR2;

	/* Unmask interrupts from EXTI2 line */
	// Relevant register: EXTI->IMR
	/*EXTI->IMR |= EXTI_IMR_MR2;

	/* Assign EXTI2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[2], or use NVIC_SetPriority
	/*NVIC_SetPriority(EXTI2_3_IRQn,0);

	/* Enable EXTI2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	/*NVIC_EnableIRQ(EXTI2_3_IRQn);
}*/


/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c *//*
void TIM2_IRQHandler()
{
	/* Check if update interrupt flag is indeed set *//*
	if ((TIM2->SR & TIM_SR_UIF) != 0)
	{
		trace_printf("\n*** Overflow! ***\n");

		/* Clear update interrupt flag */
		// Relevant register: TIM2->SR
		//TIM2->SR &= ~(TIM_SR_UIF);

		/* Restart stopped timer */
		// Relevant register: TIM2->CR1
		/*TIM2->CR1 |= TIM_CR1_CEN;
	}
}*/


/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
/*
void EXTI2_3_IRQHandler()
{
	// Declare/initialize your local variables here...
	uint32_t clockCycles = 0;
	float period = 0;
	float frequency = 0;

	/* Check if EXTI2 interrupt pending flag is indeed set *//*
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

			trace_printf("Period of the input signal: %.6f s  ", period);
			trace_printf("Frequency of the input signal: %.6f Hz\n", frequency);
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
		//
	}
}
*/
void myADC_Init() {
	RCC->AHBENR |= 0x00000200;
	GPIOA->MODER |= 0x00000c00;
	GPIOA->PUPDR |= 0x24000000;
	/* Enable ADC clock */
	RCC->APB2ENR |= 0x00000200;
	/* Calibrate ADC and wait for it to calibrate */
	ADC1->CR |= 80000000;
	while ((ADC1->CR & 80000000) != 0);
	/* Set 12-bit resolution and right data alignment for ADC */
	ADC1->CFGR1 |= 0x00000000;
	/* Enable overrun management mode*/
	ADC1->CFGR1 |= 0x00001000;
	/* Enable continuous conversion mode*/
	ADC1->CFGR1 |= 0x00002000;
	/* Set sampling time */
	ADC1->SMPR |= 0x00000000;
	/* Set channel 5 to be selected */
	ADC1->CHSELR |= 0x00000020;
	/* Enable ADC and wait for ADC ready flag to be set by hardware*/
	ADC1->CR |= 0x00000001;
	while ((ADC1->ISR & 0x00000001) == 0);
}

void myDAC_Init() {
	/* Enable DAC clock */
	RCC->APB1ENR |= 0x20000000;
	/* Set PA4 to analog */
	GPIOA->MODER |= 0x00000300;
	GPIOA->PUPDR |= 0x24000000;
	/* Enable DAC */
	DAC->CR |= 0x00000001;
}

float getADCValue() {
	/* Start ADC conversion */
	ADC1->CR |= 0x00000004;
	/* Wait for ADC conversion to finish */
	while((ADC1->ISR & 0x00000004) == 0);
	/* ADC_DR value is read and EOC flag is cleared by hardware*/
	return ((float)ADC1->DR/(float)0xFFF) * 3.3;
}

float convertADCVoltageToResistance(uint16_t voltageReading) {
	Res = (3.3 - (float)voltageReading)/(float)0xFFF;
	Res *= (float)5000;
	return Res;
}

uint16_t getDACValue(uint16_t ADCValue) {
	float vdda = 3.3 - 0.7;

	float voltage = (((float)ADCValue*vdda)/((float)4095)) + 0.7;
	float output = (voltage/3.3)*((float)4095);
	return (uint16_t)output;
}
/*UP TO HERE IS FUNCTIONS FROM TAYLORS FILE*/

void oled_Write_Cmd( unsigned char cmd )
{//added
    GPIOB->BSRR = GPIO_BSRR_BS_6;  // make PB6 = CS# = 1 set register
    GPIOB->BSRR = GPIO_BSRR_BR_7; // make PB7 = D/C# = 0 reset register
    GPIOB->BSRR = GPIO_BSRR_BR_6; // make PB6 = CS# = 0  reset register
    oled_Write( cmd );
    GPIOB->BSRR = GPIO_BSRR_BS_6; // make PB6 = CS# = 1 reset register
}

void oled_Write_Data( unsigned char data )
{//added
	//trace_printf("\nPrinting %u\n", data);
    GPIOB->BSRR = GPIO_BSRR_BS_6; // make PB6 = CS# = 1
    GPIOB->BSRR = GPIO_BSRR_BS_7; // make PB7 = D/C# = 1
    GPIOB->BSRR = GPIO_BSRR_BR_6; // make PB6 = CS# = 0
    oled_Write( data );
    GPIOB->BSRR = GPIO_BSRR_BS_6; // make PB6 = CS# = 1
}


void oled_Write( unsigned char Value )
{

	/* Wait until SPI1 is ready for writing (TXE = 1 in SPI1_SR) */

	    while((SPI1->SR & SPI_SR_TXE) == 0) {
	        //wait
	    }

	    /* Send one 8-bit character:
	       - This function also sets BIDIOE = 1 in SPI1_CR1
	    */
	    HAL_SPI_Transmit( &SPI_Handle, &Value, 1, HAL_MAX_DELAY );


	    /* Wait until transmission is complete (TXE = 1 in SPI1_SR) */

	    while((SPI1->SR & SPI_SR_TXE) == 0) {
	        //wait
	    }
}

void myGPIO_init(void) {
	// GPIOB clock enable
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	//Enable SPI clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	//init pin 4 as output
	GPIOB->MODER |= (GPIO_MODER_MODER4_0);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_4);
	GPIOB->OSPEEDR |= ~(GPIO_OSPEEDR_OSPEEDR4);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR4);

	//init pin 6
	GPIOB->MODER |= (GPIO_MODER_MODER6_0);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_6);
	GPIOB->OSPEEDR |= ~(GPIO_OSPEEDR_OSPEEDR6);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6);

	//init pin 7
	GPIOB->MODER |= (GPIO_MODER_MODER7_0);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_7);
	GPIOB->OSPEEDR |= ~(GPIO_OSPEEDR_OSPEEDR7);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR7);


}

/*
 * SCk starts low
 * RES is low and needs to be high pretty sure which i think is PB3-4 init
 */
void oled_config( void )
{

    // Don't forget to configure PB3/PB5 as AF0
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER5_1);  // Set PB3, PB5 to alternate function
	GPIOB->AFR[0] |= (0x00 << 12) | (0x00 << 20);  // SPI1 SCK on PB3, MOSI on PB5

	// Configure PB6 (CS#) and PB7 (D/C#) as outputs
    GPIOB->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0); // Set PB6, PB7 to output mode

// Don't forget to enable SPI1 clock in RCC



    SPI_Handle.Instance = SPI1;

    SPI_Handle.Init.Direction = SPI_DIRECTION_1LINE; //BIDIOE
    SPI_Handle.Init.Mode = SPI_MODE_MASTER; //MSTR
    SPI_Handle.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_Handle.Init.CLKPolarity = SPI_POLARITY_LOW; //CPOL
    SPI_Handle.Init.CLKPhase = SPI_PHASE_1EDGE; //CPHA
    SPI_Handle.Init.NSS = SPI_NSS_SOFT;
    SPI_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256; //BR[2:0]
    SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_Handle.Init.CRCPolynomial = 7;

//
// Initialize the SPI interface
//
    HAL_SPI_Init( &SPI_Handle );

//
// Enable the SPI
//
    __HAL_SPI_ENABLE( &SPI_Handle );


    /* Reset LED Display (RES# = PB4):
       - make pin PB4 = 0, wait for a few ms
       - make pin PB4 = 1, wait for a few ms
    */
    //...
    GPIOB->BRR = (1 << 4); //set bit 4 of gpio 4 to low
	//probably want a delay
    tim3_delay();

	GPIOB->BSRR = (1 << 4); //set bit 4 of gpio to high
	//probably want a delay
	tim3_delay();
//
// Send initialization commands to LED Display
//
    for ( unsigned int i = 0; i < sizeof( oled_init_cmds ); i++ )
    {
        oled_Write_Cmd( oled_init_cmds[i] );
    }


    /* Fill LED Display data memory (GDDRAM) with zeros:
       - for each PAGE = 0, 1, ..., 7
           set starting SEG = 0
           call oled_Write_Data( 0x00 ) 128 times
    */

    for (unsigned int page = 0; page < 8; page++) {
            oled_Write_Cmd(0xB0 + page);  // Set the page address
            oled_Write_Cmd(0x00);         // Set the lower column address
            oled_Write_Cmd(0x10);         // Set the higher column address
            for (unsigned int i = 0; i < 128; i++) {
                oled_Write_Data(0x00);   // Write zeros to clear the screen
            }
        }


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

	/*PA1 INIT*/
	SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI1_PA;
	EXTI->IMR |= EXTI_IMR_MR1;
	EXTI->RTSR |= EXTI_RTSR_TR1;

	NVIC_EnableIRQ(EXTI0_1_IRQn);

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
        	trace_printf("\nCURRENT STATE WAS 0, NOW 1 WHICH IS FUNC GEN\n");
        	EXTI2_3_IRQHandler();
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

	/* Check if EXTI2 interrupt pending flag is indeed set */
	if ((EXTI->PR & EXTI_PR_PR2) != 0)
	{
		//
		// 1. If this is the first edge:
		//	- Clear count register (TIM2->CNT).
		//	- Start timer (TIM2->CR1).
		if(current_state == 1) {
			if (timerTriggered == 0) {//not sure I actually need this tbh
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
				Freq = 1/period;
				timerTriggered = 1;

				trace_printf("\nVALUES FROM FUNCTION GENERATOR\n");
				trace_printf("Period of the input signal: %f\n", (uint32_t)period);
				trace_printf("Frequency of the input signal: %f\n", (uint32_t)Freq);
		}
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

void EXTI1_IRQHandler(void) {
    uint32_t clockCycles = 0;
    if ((EXTI->PR & EXTI_PR_PR1) != 0) {
        if (current_state == 0) {
            if (timerTriggered == 0) {
                timerTriggered = 1;
                TIM2->CNT = ((uint16_t)0x0000);
                TIM2->CR1 |= TIM_CR1_CEN;
            } else {
                timerTriggered = 0;
                TIM2->CR1 &= ~(TIM_CR1_CEN);
                clockCycles = TIM2->CNT;
                period = (float)clockCycles / (float)SystemCoreClock;
                Freq = 1 / period;
                timerTriggered = 1;
				
				trace_printf("\nVALUES FROM NE555\n");
                trace_printf("Period of the NE555 signal: %f\n", period);
                trace_printf("Frequency of the NE555 signal: %f\n", Freq);
            }
        }
        EXTI->PR |= EXTI_PR_PR1;
    }
}

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
// Configure the PLL for a 48MHz system clock
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



#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
