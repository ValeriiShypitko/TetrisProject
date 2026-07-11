/*
 * MAX7219_driver.h
 *
 *  Created on: Jul 4, 2026
 *      Author: polly33
 */

#ifndef IRC_MAX7219_DRIVER_H_
#define IRC_MAX7219_DRIVER_H_
#include "stm32f446xx.h"

void MAX7219_CS_LOW(void);
void MAX7219_CS_HIGH(void);
void MAX7219_Write(uint8_t addr, uint8_t data);
void MAX7219_Write_To_All(uint8_t addr, uint8_t data);
void MAX7219_Init(void);
void MAX7219_ClearDisplayMap(void);
void MAX7219_UpdateDisplay(void);
void MAX7219_AddCube(void);

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

#endif /* IRC_MAX7219_DRIVER_H_ */
