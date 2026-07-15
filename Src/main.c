/*
 * main.c
 *
 *  Created on: Jul 3, 2026
 *      Author: polly33
 */

/*
  PB5 - 4 button move right
  PB6 - 3 button spin right
  PB7 - 2 button spin left
  PB9 - 1 button move left
*/

#include "max7219_driver.h"
#include "tetris.h"
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>

extern void USART2_TX_Init(void);

extern volatile uint32_t userScore;
extern Shape_t currentShape;
extern volatile uint32_t globalTime;

extern volatile bool startRequested;
extern volatile bool gameStarted;
extern volatile bool gameOver;

extern const Shape_t shapesArr[SHAPESCOUNT];

#define MAX7219_CS_PORT GPIOB
#define MAX7219_CS_PIN GPIO_PIN_8

// PB13 = SCK, PB14 = MISO, PB15 = MOSI (AF5). NSS unused, CS is PB8 (SSM)
static void SPI2_Pins_Init(void) {
  GPIO_Handle_t spiPins;
  memset(&spiPins, 0, sizeof(spiPins));
  spiPins.GPIOx = GPIOB;
  spiPins.PinConfig.GPIOx_PinMode = GPIO_MODE_ALTFN;
  spiPins.PinConfig.GPIOx_PinAltFuncMode = 5;
  spiPins.PinConfig.GPIOx_PinOutputType = GPIO_OT_PP;
  spiPins.PinConfig.GPIOx_PinPuPdControl = GPIO_NOPUD;
  spiPins.PinConfig.GPIOx_PinSpeed = GPIO_OS_HS;

  spiPins.PinConfig.GPIOx_PinNumber = GPIO_PIN_13; // SCK
  GPIO_Init(&spiPins);

  spiPins.PinConfig.GPIOx_PinNumber = GPIO_PIN_14; // MISO
  GPIO_Init(&spiPins);

  spiPins.PinConfig.GPIOx_PinNumber = GPIO_PIN_15; // MOSI
  GPIO_Init(&spiPins);
}

static void Buttons_Init(void) {
  GPIO_Handle_t buttonInitStruct;
  memset(&buttonInitStruct, 0, sizeof(buttonInitStruct));
  buttonInitStruct.GPIOx = GPIOB;
  buttonInitStruct.PinConfig.GPIOx_PinMode = GPIO_MODE_IT_FT;
  buttonInitStruct.PinConfig.GPIOx_PinPuPdControl = GPIO_PU;
  buttonInitStruct.PinConfig.GPIOx_PinSpeed = GPIO_OS_HS;

  buttonInitStruct.PinConfig.GPIOx_PinNumber = LEFTARROW;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = RIGHTARROW;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = LEFTSPIN;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = RIGHTSPIN;
  GPIO_Init(&buttonInitStruct);
}

int main(void) {
  USART2_TX_Init();

  RCC_Enable_Clock(RCC_GPIOB);
  RCC_Enable_Clock(RCC_SPI2);

  /* PB8 = CS pin for the MAX7219 chain */
  GPIO_OutputInit(MAX7219_CS_PORT, MAX7219_CS_PIN, GPIO_OS_FS);

  Buttons_Init();
  SPI2_Pins_Init();

  /* SPI2 as master, mode 0, 8-bit, fPCLK/2, software slave management */
  SPI_Handle_t spi2;
  memset(&spi2, 0, sizeof(spi2));
  spi2.pSPIx = SPI2;
  spi2.SPIConfig.SPI_DeviceMode = SPI_DeviceMode_Master;
  spi2.SPIConfig.SPI_DFF = SPI_DFF_8Bits;
  spi2.SPIConfig.SPI_SCLKSpeed = SPI_SCLKSpeed_DIV2;
  spi2.SPIConfig.SPI_CPHA = SPI_CPHA_LEAD;
  spi2.SPIConfig.SPI_CPOL = SPI_CPOL_0;
  spi2.SPIConfig.SPI_SSM = SPI_SSM_EN;
  spi2.SPIConfig.SPI_BusConfig = SPI_BusConfig_FD;
  SPI_Init(&spi2);

  SPI_SSI_Config(SPI2);
  SPI_PeriphEnable(SPI2);

  max7219_Config(SPI2, MAX7219_CS_PORT, MAX7219_CS_PIN);
  max7219_Init();

  NVIC_EnableIRQ(IRQ_NO_EXTI9_5);

  /* PendSV is the game tick, must be the lowest priority */
  SCB_SetPendSVPriority(NVIC_PRI_15);

  SysTick_Clock_Init(1);

  clearTetrisMap();
  convertTetrisMapToDisplayMap();

  uint32_t shownScore = 0;
  bool reportedGameOver = false;

  while (!startRequested)
    ; // wait for the first button press, it also seeds the randomizer
  currentShape = shapesArr[getRandShape(globalTime)];
  gameStarted = true; // PendSV may touch currentShape from here on

  for (;;){
    if (userScore != shownScore){
      shownScore = userScore;
      printf("Your score: %lu\n", userScore);
    }

    if(gameOver && !reportedGameOver){
      reportedGameOver = true;
      printf("Game Over!\r\n");
      printf("Your final score is: %lu\n", userScore);
    }
  }
}
