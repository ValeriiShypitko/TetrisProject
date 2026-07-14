
/*
 * tetris.c
 *
 *  Created on: Jul 5, 2026
 *      Author: polly33 
 */
#include "tetris.h"
#include "max7219_driver.h"
#include "stm32f446xx_gpio_driver.h"
#include "stdlib.h"
#include "string.h"


static uint8_t tetrisSearchButton(void); // Returns the pending button pin, or NO_BUTTON if no known line is pending                          
static inline int8_t takeIntent(volatile int8_t *intent); // Returns the intent and clears it                      

const Point_t DEFAULTSPAWNPOINT = {3, 30};
const Point_t CUBESPAWNPOINT = {4, 30};

// Consts of every tetromino state in the game.
// Each state is a 4x4 bitmask, bit (i*4 + j) = cell at x = j-1, y = 1-i
// from the pivot. States advance clockwise.

// ....  ..X.  ....  .X..
// XXXX  ..X.  ....  .X..
// ....  ..X.  XXXX  .X..
// ....  ..X.  ....  .X..
const uint16_t ISHAPE_STATE[4] = {0x00F0, 0x4444, 0x0F00, 0x2222};

// X..  .XX  ...  .X.
// XXX  .X.  XXX  .X.
// ...  .X.  ..X  XX.
const uint16_t JSHAPE_STATE[4] = {0x0071, 0x0226, 0x0470, 0x0322};

// .X.  .X.  ...  .X.
// XXX  .XX  XXX  XX.
// ...  .X.  .X.  .X.
const uint16_t TSHAPE_STATE[4] = {0x0072, 0x0262, 0x0270, 0x0232};

// ..X  .X.  ...  XX.
// XXX  .X.  XXX  .X.
// ...  .XX  X..  .X.
const uint16_t LSHAPE_STATE[4] = {0x0074, 0x0622, 0x0170, 0x0223};

// .XX  .X.  ...  X..
// XX.  .XX  .XX  XX.
// ...  ..X  XX.  .X.
const uint16_t SSHAPE_STATE[4] = {0x0036, 0x0462, 0x0360, 0x0231};

// XX.  ..X  ...  .X.
// .XX  .XX  XX.  XX.
// ...  .X.  .XX  X..
const uint16_t ZSHAPE_STATE[4] = {0x0063, 0x0264, 0x0630, 0x0132};

// XX
// XX
const uint16_t OSHAPE_STATE[4] = {0x0033, 0x0033, 0x0033, 0x0033};


// Consts of every tetromino containing pivot point (x,y),default rotation number, and pointer to the rotation states
const Shape_t ISHAPE = {DEFAULTSPAWNPOINT,0, ISHAPE_STATE};
const Shape_t JSHAPE = {DEFAULTSPAWNPOINT, 0, JSHAPE_STATE};
const Shape_t LSHAPE = {DEFAULTSPAWNPOINT, 0, LSHAPE_STATE};
const Shape_t TSHAPE = {DEFAULTSPAWNPOINT, 0, TSHAPE_STATE};
const Shape_t SSHAPE = {DEFAULTSPAWNPOINT, 0, SSHAPE_STATE};
const Shape_t ZSHAPE = {DEFAULTSPAWNPOINT, 0, ZSHAPE_STATE};
const Shape_t OSHAPE = {CUBESPAWNPOINT, 0, OSHAPE_STATE};

const Shape_t shapesArr[SHAPESCOUNT] = {TSHAPE, LSHAPE, JSHAPE, OSHAPE,ISHAPE, SSHAPE, ZSHAPE};

bool tetrisMap[MAP_WIDTH][MAP_HEIGHT];

volatile uint32_t globalTime = 0;
static   uint32_t lastTimePressed = 0;

volatile uint32_t userScore = 0;


volatile int8_t queuedMove = 0;
volatile int8_t queuedSpin = 0;

Shape_t currentShape;


volatile bool startRequested = false; // set by the first button press
volatile bool gameStarted = false;    // set by main, gates the game tick
volatile bool gameOver = false;

/**
 * @brief 7-bag randomizer: deals all 7 tetrominoes before any repeats.
 *
 * @note  no retry loop, safe to call from PendSV
 */
uint8_t getRandShape(uint32_t seed) {
  static uint8_t bag[SHAPESCOUNT] = {0, 1, 2, 3, 4, 5, 6};
  static uint8_t remaining = 0;

  if (remaining == 0) {
    for (uint8_t i = SHAPESCOUNT - 1; i > 0; i--) {
      uint8_t j = (uint8_t)((rand() + seed) % (i + 1));
      uint8_t tmp = bag[i];
      bag[i] = bag[j];
      bag[j] = tmp;
    }
    remaining = SHAPESCOUNT;
  }
  return bag[--remaining];
}

/**
 * @brief  adds current piece to the tetrisMap and refreshes the display 
 */
void addPiece(Shape_t *shape) {
  for (uint8_t i = 0; i < SHAPE_LEN; i++) {
    for (uint8_t j = 0; j < SHAPE_LEN; j++) {
      if (shape->shape[shape->rotateNum]  & (1U << (j + i * SHAPE_LEN))) {
        tetrisMap[shape->pivot.x  + j - 1][shape->pivot.y - i + 1] = 1;
      }
    }
  }
  convertTetrisMapToDisplayMap();  // refresh display after adding the piece(tetromino)
}

/**
 * @brief  returns true if given shape can be placed at given position and rotation
 *
 * @note   tetrisMap never contains the falling piece itself (addPiece() is
 *         always paired with clearPiece()), so no self-collision is possible.
 */

static bool canPlace(const Shape_t *shape, uint8_t rot, int8_t px, int8_t py) {
  for (uint8_t i = 0; i < SHAPE_LEN; i++) {
    for (uint8_t j = 0; j < SHAPE_LEN; j++) {
      
      if ((shape->shape[rot] & (1U << (j + i * SHAPE_LEN)))== 0)
        continue;

      int8_t x = px + j - 1;
      int8_t y = py - i + 1;

      if (x < 0 || x > 7 || y < 0 || y > 31)
        return false;
      if (tetrisMap[x][y] == 1)
        return false;
    }
  }
  return true;
}

bool canSpawn(Shape_t *shape) {
  return canPlace(shape, shape->rotateNum, shape->pivot.x, shape->pivot.y);
}

bool canGoDown(Shape_t *shape) {
  // down is -y
  return canPlace(shape, shape->rotateNum, shape->pivot.x, shape->pivot.y - 1);
}

bool canGoLeft(Shape_t *shape) {
  return canPlace(shape, shape->rotateNum, shape->pivot.x - 1, shape->pivot.y);
}

bool canGoRight(Shape_t *shape) {
  return canPlace(shape, shape->rotateNum, shape->pivot.x + 1, shape->pivot.y);
}

bool canSpin_Left(Shape_t *shape) {
  return canPlace(shape, (shape->rotateNum + 3) % 4, shape->pivot.x,
                  shape->pivot.y);
}

bool canSpin_Right(Shape_t *shape) {
  return canPlace(shape, (shape->rotateNum + 1) % 4, shape->pivot.x,
                  shape->pivot.y);
}


/**
 * @brief removes all full rows from the tetrisMap and shifts everything above down, updating the user score accordingly
 *
 * @note doesn't refresh the display  
 *        
 */
void removeFullRows(void) {
  for (uint8_t i = 0; i < MAP_HEIGHT;) {
    uint8_t countTheRow = 0;
    for (uint8_t j = 0; j < MAP_WIDTH; j++) {
      if (tetrisMap[j][i] == 1)
        countTheRow++;
    }
    if (countTheRow == MAP_WIDTH) {
      userScore += SCORE_PER_ROW;

      for (uint8_t k = i + 1; k < MAP_HEIGHT; k++) {
        for (uint8_t j = 0; j < MAP_WIDTH; j++) {
          tetrisMap[j][k - 1] = tetrisMap[j][k];
        }
      }

      for (uint8_t j = 0; j < MAP_WIDTH; j++) {
        tetrisMap[j][MAP_HEIGHT - 1] = 0;
      }
      continue; 
    }
    i++;
  }
}

/**
 * @brief clears the current piece from the tetrisMap 
 *
 * @note doesn't refresh the display  
 *        
 */
void clearPiece(Shape_t *shape) {
  for (uint8_t i = 0; i < SHAPE_LEN; i++) {
    for (uint8_t j = 0; j < SHAPE_LEN; j++) {
      if (shape->shape[shape->rotateNum] & (1U << (j + i * SHAPE_LEN)))
        tetrisMap[shape->pivot.x + j - 1][shape->pivot.y - i + 1] = 0;
    }
  }
}

/**
 * @brief clears the entire tetrisMap


   @note doesn't refresh the display
 */
void clearTetrisMap(void) { memset(tetrisMap, 0, sizeof(tetrisMap)); }

/**
 * @brief moves the pivot of the current piece down by 1 unit in the y direction
 */
void movePieceDown(Shape_t *shape) { shape->pivot.y -= 1; }


/**
 * @brief converts the tetrisMap to display_Map and refreshes the display
 */
void convertTetrisMapToDisplayMap(void) {
  max7219_ClearDisplayMap();
  for (uint8_t i = 0; i < MAP_HEIGHT; i++) {
    uint8_t displayNum = (31 - i) / MAP_WIDTH;
    for (uint8_t j = 0; j < 8; j++) {
      display_Map[displayNum][7 - j] |= (tetrisMap[j][i] << (i % MAP_WIDTH));
    }
  }
  max7219_UpdateDisplay();
}


/**
 * @brief searches for the pending button pin in EXTI->PR and returns it, or NO_BUTTON if no known line is pending
 */
static uint8_t tetrisSearchButton(void){
  static const uint8_t buttonPins[BUTTONCOUNT] = {LEFTARROW, RIGHTARROW,
                                                  LEFTSPIN, RIGHTSPIN};
  for (uint8_t i = 0; i < BUTTONCOUNT; i++){
    if(EXTI->PR & (1U << buttonPins[i]))
      return buttonPins[i];
  }
  return NO_BUTTON;
}

// Atomic read-and-clear of a queued intent
static inline int8_t takeIntent(volatile int8_t *slot) {
  __asm volatile("CPSID i" ::: "memory");
  int8_t v = *slot;
  *slot = 0;
  __asm volatile("CPSIE i" ::: "memory");
  return v;
}


/**
 * @brief pends a PendSV exception to the NVIC, which will be handled after the current interrupt and all higher-priority interrupts have completed
 */
void pendPendSV(void){ SCB->ICSR = SCB_ICSR_PENDSVSET; }


/**
 * @brief queues a move or spin intent based on the button pressed, with debouncing
 */
void EXTI9_5_IRQHandler(void){
  uint8_t pin = tetrisSearchButton();
  if(pin == NO_BUTTON){
    // not our line; leave the button lines alone, a press could be pending
    EXTI->PR = EXTI9_5_LINES & ~BUTTON_LINES_MASK;
    return;
  }
  GPIO_IRQHandling(pin); 

  if(globalTime - lastTimePressed < DEBOUNCE_MS)
    return;
  lastTimePressed = globalTime;

  if(gameStarted){
    switch (pin) { 
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
  // main picks the first shape, then sets gameStarted itself
  startRequested = true;
}

/**
 * @brief handles the SysTick interrupt and game frames at a fixed interval defined by GAMESPEED
*/
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

/**
 * @brief handles the PendSV interrupt and updates the game state
*
*
*  @note this is where the game logic is executed, including moving and spinning the current shape, checking for collisions, and updating the score
*/
void PendSV_Handler(void){
  int8_t mv = takeIntent(&queuedMove);
  int8_t sp = takeIntent(&queuedSpin);

  // a move and a spin queued in the same tick both apply
  if(mv < 0 && canGoLeft(&currentShape))
    currentShape.pivot.x--;
  else if(mv > 0 && canGoRight(&currentShape))
    currentShape.pivot.x++;

  if(sp < 0 && canSpin_Left(&currentShape))
    currentShape.rotateNum = (currentShape.rotateNum + 3) % 4;
  else if(sp > 0 && canSpin_Right(&currentShape))
    currentShape.rotateNum = (currentShape.rotateNum + 1) % 4;

  if(canGoDown(&currentShape)){
    movePieceDown(&currentShape);
    addPiece(&currentShape);   // draw
    clearPiece(&currentShape); // map keeps only settled blocks
  }else{
    addPiece(&currentShape);   // lock
    removeFullRows();
    convertTetrisMapToDisplayMap();
    currentShape = shapesArr[getRandShape(globalTime)];
    if(canSpawn(&currentShape) == false){
      SysTick_Clock_DeInit();
      NVIC_DisableIRQ(IRQ_NO_EXTI9_5);
      max7219_ClearDisplayMap();
      max7219_UpdateDisplay();
      gameOver = true;
      return;
    }
  }
  userScore += SCORE_PER_TICK;

}