/*
 * max7219_driver.h
 *
 *  Created on: Jul 4, 2026
 *      Author: polly33
 */

#ifndef INC_MAX7219_DRIVER_H_
#define INC_MAX7219_DRIVER_H_
#include "stm32f446xx.h"

typedef struct {
  SPI_RegDef_t *pSPIx;
  GPIOx_RegDef_t *csPort;
  uint8_t csPin;
} max7219_Handle_t;

void max7219_Config(SPI_RegDef_t *pSPIx, GPIOx_RegDef_t *csPort,
                    uint8_t csPin);
void max7219_CS_LOW(void);
void max7219_CS_HIGH(void);
void max7219_Write(uint8_t addr, uint8_t data);
void max7219_Write_To_All(uint8_t addr, uint8_t data);
void max7219_Init(void);
void max7219_ClearDisplayMap(void);
void max7219_UpdateDisplay(void);

#define MAX7219_ADDR_NO_OP 0x0
#define MAX7219_ADDR_DIG_0 0x1
#define MAX7219_ADDR_DIG_1 0x2
#define MAX7219_ADDR_DIG_2 0x3
#define MAX7219_ADDR_DIG_3 0x4
#define MAX7219_ADDR_DIG_4 0x5
#define MAX7219_ADDR_DIG_5 0x6
#define MAX7219_ADDR_DIG_6 0x7
#define MAX7219_ADDR_DIG_7 0x8
#define MAX7219_ADDR_DEC_MOD 0x9
#define MAX7219_ADDR_INTENSITY 0xA
#define MAX7219_ADDR_SCAN_LIM 0xB
#define MAX7219_ADDR_SHUTDOWN 0xC
#define MAX7219_ADDR_DISPLAY_TEST 0xF

#define NUM_OF_DISPLAYS 4

extern uint8_t display_Map[NUM_OF_DISPLAYS][8];

#endif /* INC_MAX7219_DRIVER_H_ */
