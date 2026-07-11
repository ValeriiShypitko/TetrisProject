/*
 * Tetris.h
 *
 *
 *  Created on: Jul 6, 2026
 *      Author: polly33
 */

#ifndef INC_TETRIS_H_
#define INC_TETRIS_H_
#include "MAX7219_driver.h"
#include "stdbool.h"
#include "stm32f446xx.h"

#define SHAPE_LEN_2 0x2
#define SHAPE_LEN_3 0x3
#define SHAPE_LEN_4 0x4
#define SHAPE_DEFAULT_LENGTH 0x4
#define SHAPESCOUNT 0x7
#define GAMESPEED 270U
#define MAPHIGHT 32U
#define MAPWIDTH 8U
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
void convertTetrisMapToDisplayMap(void);
void clearTetrisMap(void);
void spawnFigure(void);
void clearFigure(Shape_t *shape);
void addFigure(Shape_t *shape);
void removeFullRows(void);
// Hit check
uint8_t getRandShape(uint32_t seed);
bool canSpawn(Shape_t *shape);
bool canGoDown(Shape_t *shape);
bool canGoLeft(Shape_t *shape);
bool canGoRight(Shape_t *shape);
bool canSpin_Left(Shape_t *shape);
bool canSpin_Right(Shape_t *shape);
void moveFigureDown(Shape_t *shape);
#endif /* INC_TETRIS_H_ */
