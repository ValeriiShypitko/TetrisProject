/*
 * tetris.h 
 *  Created on: Jul 6, 2026
 *      Author: polly33
 */

#ifndef INC_TETRIS_H_
#define INC_TETRIS_H_
#include "max7219_driver.h"
#include "stdbool.h"
#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"


typedef struct {
  int8_t x;
  int8_t y;
  uint8_t value;
} Point_Value_t;

typedef struct {
  int8_t x;
  int8_t y;
} Point_t;

typedef struct {
  Point_t pivot;
  uint8_t len;
  uint8_t rotateNum;
  const Point_Value_t (*shape)[4][4];
} Shape_t;

#define SHAPE_LEN_2 0x2
#define SHAPE_LEN_3 0x3
#define SHAPE_LEN_4 0x4
#define SHAPESCOUNT 0x7
#define GAMESPEED 270U
#define MAP_HEIGHT 32U
#define MAP_WIDTH 8U

#define BUTTONCOUNT 4U
#define LEFTARROW GPIO_PIN_9
#define RIGHTARROW GPIO_PIN_5
#define LEFTSPIN GPIO_PIN_7
#define RIGHTSPIN GPIO_PIN_6
#define NO_BUTTON 0xFFU

#define EXTI9_5_LINES 0x03E0U
#define BUTTON_LINES_MASK                                                      \
  ((1U << LEFTARROW) | (1U << RIGHTARROW) | (1U << LEFTSPIN) |                 \
   (1U << RIGHTSPIN))
#define DEBOUNCE_MS 120U

#define SCORE_PER_ROW 1100U
#define SCORE_PER_TICK 75U




void convertTetrisMapToDisplayMap(void);
void clearTetrisMap(void);
void clearPiece(Shape_t *shape);
void addPiece(Shape_t *shape);
void removeFullRows(void);
uint8_t getRandShape(uint32_t seed);

// Hit check

bool canSpawn(Shape_t *shape);
bool canGoDown(Shape_t *shape);
bool canGoLeft(Shape_t *shape);
bool canGoRight(Shape_t *shape);
bool canSpin_Left(Shape_t *shape);
bool canSpin_Right(Shape_t *shape);

void movePieceDown(Shape_t *shape);

#endif /* INC_TETRIS_H_ */
