/*
 * max7219_driver.c
 *
 *  Created on: Jul 4, 2026
 *      Author: polly33
 */
#include "max7219_driver.h"
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include <stdint.h>
#include <string.h>

uint8_t display_Map[NUM_OF_DISPLAYS][8];

// set via max7219_Config() before use
static max7219_Handle_t hmax7219;

void max7219_Config(SPI_RegDef_t *pSPIx, GPIOx_RegDef_t *csPort,
                    uint8_t csPin) {
  hmax7219.pSPIx = pSPIx;
  hmax7219.csPort = csPort;
  hmax7219.csPin = csPin;
  max7219_CS_HIGH(); // CS idles high
}

void max7219_CS_LOW(void) { SPI_Pull_CS_LOW(hmax7219.csPort, hmax7219.csPin); }
void max7219_CS_HIGH(void) {
  SPI_Pull_CS_HIGH(hmax7219.csPort, hmax7219.csPin);
}

void max7219_Write(uint8_t addr, uint8_t data) {
  SPI_SendData(hmax7219.pSPIx, &addr, 1);
  SPI_SendData(hmax7219.pSPIx, &data, 1);
}

/**
 * @brief Sends the same command to every device in the daisy chain.
 *
 * @note  Repeating the frame N times shifts it through the chain, so after
 *        the last iteration every device holds the same command.
 */
void max7219_Write_To_All(uint8_t addr, uint8_t data) {
  for (uint8_t i = 0; i < NUM_OF_DISPLAYS; i++) {
    max7219_CS_LOW();
    max7219_Write(addr, data);
    max7219_CS_HIGH();
  }
}

void max7219_Init(void) {
  max7219_Write_To_All(MAX7219_ADDR_SHUTDOWN, 0x0);
  max7219_Write_To_All(MAX7219_ADDR_SCAN_LIM, 0x7);
  max7219_Write_To_All(MAX7219_ADDR_INTENSITY, 0x1);
  max7219_Write_To_All(MAX7219_ADDR_DISPLAY_TEST, 0x0);
  max7219_Write_To_All(MAX7219_ADDR_SHUTDOWN, 0x01);
}

/**
 * @brief Pushes display_Map to the displays, one digit row at a time.
 *
 * @note  All 4 frames of a row go out in one CS window, so each device
 *        latches its own frame on the rising CS edge.
 */
void max7219_UpdateDisplay(void) {
  for (uint8_t i = 0; i < 8; i++) {
    max7219_CS_LOW();
    for (uint8_t j = 0; j < NUM_OF_DISPLAYS; j++) {
      max7219_Write(i + 1, display_Map[j][i]);
    }
    max7219_CS_HIGH();
  }
}

void max7219_ClearDisplayMap(void) {
  memset(display_Map, 0, sizeof(display_Map));
}
