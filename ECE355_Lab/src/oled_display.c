#include "oled_display.h"
#include <stdio.h>

unsigned int Freq = 0;
unsigned int Res = 0;

SPI_HandleTypeDef SPI_Handle;

unsigned char oled_init_cmds[] = {
    0xAE, 0x20, 0x00, 0x40, 0xA1, 0xA8, 0x3F, 0xC8, 0xD3, 0x00,
    0xDA, 0x12, 0x81, 0x7F, 0xA4, 0xA6, 0xD5, 0x80, 0x8D, 0x14,
    0xAF
};

unsigned char Characters[][8] = {
    // Add character bitmaps here
};

void oled_Write(unsigned char value) {
    while ((SPI1->SR & SPI_SR_TXE) == 0);
    HAL_SPI_Transmit(&SPI_Handle, &value, 1, HAL_MAX_DELAY);
    while ((SPI1->SR & SPI_SR_TXE) == 0);
}

void oled_Write_Cmd(unsigned char cmd) {
    GPIOB->BSRR = GPIO_BSRR_BS_6;
    GPIOB->BSRR = GPIO_BSRR_BR_7;
    GPIOB->BSRR = GPIO_BSRR_BR_6;
    oled_Write(cmd);
    GPIOB->BSRR = GPIO_BSRR_BS_6;
}

void oled_Write_Data(unsigned char data) {
    GPIOB->BSRR = GPIO_BSRR_BS_6;
    GPIOB->BSRR = GPIO_BSRR_BS_7;
    GPIOB->BSRR = GPIO_BSRR_BR_6;
    oled_Write(data);
    GPIOB->BSRR = GPIO_BSRR_BS_6;
}

void oled_config(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER5_1);
    GPIOB->AFR[0] |= (0x00 << 12) | (0x00 << 20);
    GPIOB->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    SPI_Handle.Instance = SPI1;
    SPI_Handle.Init.Direction = SPI_DIRECTION_1LINE;
    SPI_Handle.Init.Mode = SPI_MODE_MASTER;
    SPI_Handle.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_Handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_Handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_Handle.Init.NSS = SPI_NSS_SOFT;
    SPI_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_Handle.Init.CRCPolynomial = 7;

    HAL_SPI_Init(&SPI_Handle);
    __HAL_SPI_ENABLE(&SPI_Handle);

    GPIOB->BRR = (1 << 4);
    HAL_Delay(10);
    GPIOB->BSRR = (1 << 4);
    HAL_Delay(10);

    for (unsigned int i = 0; i < sizeof(oled_init_cmds); i++) {
        oled_Write_Cmd(oled_init_cmds[i]);
    }

    for (int i = 0; i < 8; i++) {
        oled_Write_Cmd(0xB0 + i);
        oled_Write_Cmd(0x00);
        oled_Write_Cmd(0x10);
        for (int j = 0; j < 128; j++) {
            oled_Write_Data(0x00);
        }
    }
}

void refresh_OLED(void) {
    unsigned char Buffer[17];

    snprintf(Buffer, sizeof(Buffer), "R: %5u Ohms", Res);
    oled_Write_Cmd(0xB2);
    oled_Write_Cmd(0x03);
    oled_Write_Cmd(0x00);

    for (unsigned int i = 0; i < sizeof(Buffer) && Buffer[i] != '\0'; i++) {
        unsigned char c = Buffer[i];
        for (int j = 0; j < 8; j++) {
            oled_Write_Data(Characters[c][j]);
        }
    }

    snprintf(Buffer, sizeof(Buffer), "F: %5u Hz", Freq);
    oled_Write_Cmd(0xB3);
    oled_Write_Cmd(0x03);
    oled_Write_Cmd(0x00);

    for (unsigned int i = 0; i < sizeof(Buffer) && Buffer[i] != '\0'; i++) {
        unsigned char c = Buffer[i];
        for (int j = 0; j < 8; j++) {
            oled_Write_Data(Characters[c][j]);
        }
    }

    TIM3->CNT = 0;
    while (TIM3->CNT < 100);
}