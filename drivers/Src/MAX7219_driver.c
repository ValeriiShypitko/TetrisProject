/*
 * MAX7219_driver.c
 *
 *  Created on: Jul 4, 2026
 *      Author: polly33
 */
#include "../Inc/MAX7219_driver.h"
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include <stdint.h>
#include <string.h>
uint8_t Display_Map[4][8];

void MAX7219_CS_LOW(void) { SPI_Pull_CS_LOW(GPIOB, GPIO_PIN_8); }
void MAX7219_CS_HIGH(void) { SPI_Pull_CS_HIGH(GPIOB, GPIO_PIN_8); }

void MAX7219_Write(uint8_t addr, uint8_t data) {
  SPI_SendData(SPI2, &addr, 1);
  SPI_SendData(SPI2, &data, 1);
}
void MAX7219_Write_To_All(uint8_t addr, uint8_t data) {
  for (uint8_t i = 0; i < NUM_OF_DISPLAYS; i++) {
    MAX7219_CS_LOW();
    MAX7219_Write(addr, data);
    MAX7219_CS_HIGH();
  }
}

void MAX7219_Init(void) {
  MAX7219_CS_LOW();
  MAX7219_Write_To_All(MAX7219_ADDR_SHUTDOWN, 0x0);
  MAX7219_Write_To_All(MAX7219_ADDR_DEC_MOD, 0x0);
  MAX7219_Write_To_All(MAX7219_ADDR_SCAN_LIM, 0x7);
  MAX7219_Write_To_All(MAX7219_ADDR_INTENSITY, 0x1);
  MAX7219_Write_To_All(MAX7219_ADDR_DISPLAY_TEST, 0x0);
  MAX7219_Write_To_All(MAX7219_ADDR_SHUTDOWN, 0x01);
  MAX7219_CS_HIGH();
}

void MAX7219_UpdateDisplay(void) {
  for (uint8_t i = 0; i < 8; i++) {
    MAX7219_CS_LOW();
    for (uint8_t j = 0; j < 4; j++) {
      MAX7219_Write(i + 1, Display_Map[j][i]);
    }
    MAX7219_CS_HIGH();
  }
}

void MAX7219_ClearDisplayMap(void) { memset(Display_Map, 0, sizeof(Display_Map)); }
