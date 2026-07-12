/*
 * testing.c
 *
 *  Created on: Jul 3, 2026
 *      Author: polly33
 */

/*
  PB5 - 4 button
  PB6 - 3 button
  PB7 - 2 button
  PB9 - 1 button
*/

#include "MAX7219_driver.h"
#include "Tetris.h"
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BUTTONCOUNT 4U
#define LEFTARROW GPIO_PIN_9
#define RIGHTARROW GPIO_PIN_5
#define LEFTSPIN GPIO_PIN_7
#define RIGHTSPIN GPIO_PIN_6

#define NO_BUTTON 0xFFU
/* EXTI lines 5..9 -- everything EXTI9_5_IRQHandler is responsible for. */
#define EXTI9_5_LINES 0x03E0U
/* A real contact-bounce window. The old 310 ms was not a debounce but a rate
 * limiter, needed because a press cost up to two 300 ms game ticks to apply.
 * Raise this if the buttons still double-trigger. */
#define DEBOUNCE_MS 120U

#define PendSV_SysTick 0x1
#define PendSV_EXTI 0x0

const Point_t DEFAULTSPAWNPOINT = {3, 30};

/* Written by an ISR, read by another context: the compiler must not cache
 * these in registers or hoist the reads out of a loop. */
static volatile uint32_t globalTime = 0;
uint8_t whoCalledPendSV;

bool calledSysTick = false;
bool calledEXTI = false;

/* Input intent, recorded by EXTI9_5_IRQHandler and consumed by PendSV_Handler.
 * -1 or +1, 0 meaning nothing queued. Deliberately not validated at capture
 * time: PendSV tests them against the board at the instant it applies them. */
volatile int8_t queuedMove = 0;
volatile int8_t queuedSpin = 0;
static uint32_t lastTimePressed = 0;

/**
 * @brief Read an intent slot and clear it as one indivisible step.
 * @Note  PendSV is the lowest-priority exception, so EXTI can preempt it. A
 *        plain read-then-clear would silently drop a press that lands in
 *        between the two.
 */
static inline int8_t takeIntent(volatile int8_t *slot) {
  __asm volatile("CPSID i" ::: "memory");
  int8_t v = *slot;
  *slot = 0;
  __asm volatile("CPSIE i" ::: "memory");
  return v;
}
static uint8_t
TetrisSearchButton(void); // Returns the pending button pin, or
                          // NO_BUTTON if no known line is pending

const Point_Value_t IShape_Rts[4][4][4] = {
    {{{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {2, 1, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {2, 0, 1}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {2, -1, 0}},
     {{-1, -2, 0}, {0, -2, 0}, {1, -2, 0}, {2, -2, 0}}},
    {{{-1, 1, 0}, {0, 1, 0}, {1, 1, 1}, {2, 1, 0}},
     {{-1, 0, 0}, {0, 0, 0}, {1, 0, 1}, {2, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 1}, {2, -1, 0}},
     {{-1, -2, 0}, {0, -2, 0}, {1, -2, 1}, {2, -2, 0}}},
    {{{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {2, 1, 0}},
     {{-1, 0, 0}, {0, 0, 0}, {1, 0, 0}, {2, 0, 0}},
     {{-1, -1, 1}, {0, -1, 1}, {1, -1, 1}, {2, -1, 1}},
     {{-1, -2, 0}, {0, -2, 0}, {1, -2, 0}, {2, -2, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {2, 1, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {2, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {2, -1, 0}},
     {{-1, -2, 0}, {0, -2, 1}, {1, -2, 0}, {2, -2, 0}}}

};
const Point_Value_t JShape_Rts[4][4][4] = {
    {{{-1, 1, 1}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 1}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
        {{-1, -1, 0}, {0, -1, 0}, {1, -1, 1}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 1}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}

};
const Point_Value_t TShape_Rts[4][4][4] = {
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
        {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}};
const Point_Value_t LShape_Rts[4][4][4] = {
    {{{-1, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 1}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
        {{-1, -1, 1}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}};
const Point_Value_t SShape_Rts[4][4][4] = {
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 1}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 1}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
        {{-1, -1, 1}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 1}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}};
const Point_Value_t ZShape_Rts[4][4][4] = {
    {{{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 0, 0}},
     {{-1, 0, 0}, {0, 0, 1}, {1, 0, 1}, {0, 0, 0}},
     {{-1, -1, 0}, {0, -1, 1}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
        {{-1, -1, 0}, {0, -1, 1}, {1, -1, 1}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 0}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{-1, -1, 1}, {0, -1, 0}, {1, -1, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}};

const Point_Value_t OShape_Rts[4][4][4] = {
    {{{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {

        {{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
        {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}},
    {{{-1, 1, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}},
     {{-1, 0, 1}, {0, 0, 1}, {1, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}}};
const Point_t CUBESPAWNPOINT = {4, 30};

const Shape_t IShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_4, 0, IShape_Rts};

const Shape_t JShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_3, 0, JShape_Rts};
const Shape_t LShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_3, 0, LShape_Rts};
const Shape_t TShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_3, 0, TShape_Rts};
const Shape_t SShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_3, 0, SShape_Rts};
const Shape_t ZShape = {DEFAULTSPAWNPOINT, SHAPE_LEN_3, 0, ZShape_Rts};

const Shape_t OShape = {CUBESPAWNPOINT, SHAPE_LEN_2, 0, OShape_Rts};

Shape_t currentShape;

Shape_t shapesArr[SHAPESCOUNT] = {TShape, LShape, JShape, OShape,
                                  IShape, SShape, ZShape};

extern volatile uint32_t userScore;

/* Raised by PendSV, reported by the foreground. */
volatile bool gameStarted = false;
volatile bool gameOver = false;

extern void USART2_TX_Init(void);

int main(void) {
  USART2_TX_Init();
  RCC_Enable_Clock(RCC_GPIOB);
  RCC_Enable_Clock(RCC_SPI2);

  /* PB8 = CS PIN */
  GPIO_OutputInit(GPIOB, GPIO_PIN_8, GPIO_OS_FS);
  GPIO_Handle_t buttonInitStruct;
  memset(&buttonInitStruct, 0, sizeof(buttonInitStruct));

  buttonInitStruct.GPIOx = GPIOB;
  buttonInitStruct.PinConfig.GPIOx_PinMode = GPIO_MODE_IT_FT;
  buttonInitStruct.PinConfig.GPIOx_PinPuPdControl = GPIO_PU;
  buttonInitStruct.PinConfig.GPIOx_PinNumber = LEFTARROW;
  buttonInitStruct.PinConfig.GPIOx_PinSpeed = GPIO_OS_HS;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = RIGHTARROW;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = LEFTSPIN;
  GPIO_Init(&buttonInitStruct);

  buttonInitStruct.PinConfig.GPIOx_PinNumber = RIGHTSPIN;
  GPIO_Init(&buttonInitStruct);

  SPI2_GPIOs_Init();
  SPI_Pull_CS_HIGH(GPIOB, GPIO_PIN_12); // CS idle high before any transfer
  SPI_Pull_CS_HIGH(GPIOC, GPIO_PIN_11);
  /* SPI2 as master, mode 0, 8-bit, fPCLK/2, software slave management.   */
  SPI_Handle_t spi2;
  spi2.pSPIx = SPI2;
  spi2.SPIConfig.SPI_DeviceMode = SPI_DeviceMode_Master;
  spi2.SPIConfig.SPI_DFF = SPI_DFF_8Bits;
  spi2.SPIConfig.SPI_SCLKSpeed = SPI_SCLKSpeed_DIV2;
  spi2.SPIConfig.SPI_CPHA = SPI_CPHA_LEAD; /* CPHA = 0 */
  spi2.SPIConfig.SPI_CPOL = SPI_CPOL_0;    /* CPOL = 0  -> SPI mode 0   */
  spi2.SPIConfig.SPI_SSM = SPI_SSM_EN;     /* software-managed NSS      */

  spi2.SPIConfig.SPI_BusConfig = SPI_BusConfig_FD;
  SPI_Init(&spi2);
  /* With SSM=1 in master mode, SSI must be forced high or the peripheral
   * sees NSS low and faults with MODF. Must be set before enabling SPE.  */

  SPI2_SSI_Config(SPI2);

  /* Enable the SPI peripheral (SPE=1) so transfers can run.              */
  SPI_PeriphEnable(SPI2);

  MAX7219_Init();

  NVIC_EnableIRQ(IRQ_NO_EXTI9_5);

  /* PendSV is the game tick: it is long-running and never urgent. Park it at
   * the lowest priority so SysTick keeps counting milliseconds and EXTI keeps
   * catching presses while it runs. */
  SCB_SetPendSVPriority(NVIC_PRI_15);

  SysTick_Clock_Init(1);

  clearTetrisMap();
  convertTetrisMapToDisplayMap();

  /* Timekeeping lives in SysTick, input capture in EXTI, and the game tick in
   * PendSV. The foreground only reports, because printf() over semihosting
   * halts the core for milliseconds and must never run inside an exception. */
  uint32_t shownScore = 0;
  bool reportedGameOver = false;

  while (!gameStarted)
    ; // Wait until user presses button
  currentShape = shapesArr[getRandShape(globalTime)];
  for (;;){
    uint32_t score = userScore;
    if (score != shownScore){
      shownScore = score;
      printf("Your score: %lu\n", score);
    }

    if(gameOver && !reportedGameOver){
      reportedGameOver = true;
      printf("Game Over!\r\n");
      /* re-read: PendSV publishes the score before it raises gameOver */
      printf("Your final score is: %lu\n", userScore);
    }
  }
  return 0;
}

static uint8_t TetrisSearchButton(void){
  static const uint8_t buttonPins[BUTTONCOUNT] = {LEFTARROW, RIGHTARROW,
                                                  LEFTSPIN, RIGHTSPIN};
  for (uint8_t i = 0; i < BUTTONCOUNT; i++){
    if(EXTI->PR & (1U << buttonPins[i]))
      return buttonPins[i];
  }
  return NO_BUTTON;
}

/* PENDSVSET is write-1-to-set; a read-modify-write would also write back any
 * other set bits it happened to read (PENDSTSET among them). Plain store. */
void pendPendSV(void){ SCB->ICSR = (1U << 28);}

void EXTI9_5_IRQHandler(void){
  uint8_t pin = TetrisSearchButton();
  if(pin == NO_BUTTON){
    /* A line we don't own is pending. Acknowledge it, or we re-enter forever.*/
    EXTI->PR = EXTI9_5_LINES;
    return;
  }
  GPIO_IRQHandling(pin); /* acknowledge only the line we handled */

  /* Drop the press before it is published, so a contact bounce cannot queue
   * a second move. */

  if(globalTime - lastTimePressed < DEBOUNCE_MS)
    return;
  lastTimePressed = globalTime;

  if(gameStarted){
    switch (pin) { /* record the intent; PendSV decides whether it is legal */

    case LEFTARROW:
      queuedMove = -1;
      break;
    case RIGHTARROW:
      queuedMove = +1;
      break;
    case LEFTSPIN:
      queuedSpin = -1;
      break;
    case RIGHTSPIN:
      queuedSpin = +1;
      break;
    default:
      break;
    }
  }
  gameStarted = true;
}

void SysTick_Handler(void){
  static uint32_t updateScreenTime = 0;

  globalTime++;
  if(gameStarted){
    updateScreenTime++;
    if(updateScreenTime >= GAMESPEED) {
      updateScreenTime = 0;
      pendPendSV();
    }
  }
}

void PendSV_Handler(void){
  int8_t mv = takeIntent(&queuedMove);
  int8_t sp = takeIntent(&queuedSpin);

  /* Validated here, against the board as it stands right now. A move and a
   * rotation queued in the same tick both apply. */
  if(mv < 0 && canGoLeft(&currentShape))
    currentShape.pivot.x--;
  else if(mv > 0 && canGoRight(&currentShape))
    currentShape.pivot.x++;

  if(sp < 0 && canSpin_Left(&currentShape))
    currentShape.rotateNum = (currentShape.rotateNum + 3) % 4;
  else if(sp > 0 && canSpin_Right(&currentShape))
    currentShape.rotateNum = (currentShape.rotateNum + 1) % 4;

  bool canDown = canGoDown(&currentShape);

  if(canDown){

    addFigure(&currentShape);
    clearFigure(&currentShape);
    moveFigureDown(&currentShape);
  }else{
    currentShape.pivot.y = currentShape.pivot.y + 1;
    addFigure(&currentShape);
    removeFullRows();
    currentShape = shapesArr[getRandShape(globalTime)];
    if(canSpawn(&currentShape) == false){
      SysTick_Clock_DeInit(); 
      NVIC_DisableIRQ(IRQ_NO_EXTI9_5); 
      MAX7219_ClearDisplayMap();
      MAX7219_UpdateDisplay();
      gameOver = true; 
      return;          
    }
  }
  userScore += 75;
}
