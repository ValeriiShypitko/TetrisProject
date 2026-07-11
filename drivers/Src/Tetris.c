#include "Tetris.h"
#include "MAX7219_driver.h"
#include "stdlib.h"
bool TetrisMap[MAPWIDTH][MAPHIGHT];

extern uint8_t Display_Map[4][8];

static uint32_t rng_state = 123456789;
/* Updated inside PendSV, polled by the foreground reporter. */
volatile uint32_t userScore = 0;
uint8_t getRandShape(uint32_t seed) {
  static bool usedShapes[7] = {0, 0, 0, 0, 0, 0, 0};
  static uint8_t usedShapesCount = 0;
  uint8_t result;
  while (true) {
    result = (rand() + seed) % SHAPESCOUNT;
    if (usedShapes[result] == false)
      break;
  }
  usedShapes[result] = true;
  usedShapesCount++;
  if (usedShapesCount >= 7) {
    memset(usedShapes, 0, sizeof(usedShapes));
    usedShapesCount = 0;
  }
  return result;
}

void addFigure(Shape_t *shape) {
  for (uint8_t i = 0; i < shape->len; i++) {
    for (uint8_t j = 0; j < shape->len; j++) {
      if (shape->shape[shape->rotateNum][i][j].value == 1) {
        TetrisMap[shape->pivot.x + shape->shape[shape->rotateNum][i][j].x]
                 [shape->pivot.y + shape->shape[shape->rotateNum][i][j].y] = 1;
      }
    }
  }
  convertTetrisMapToDisplayMap(); /* one refresh, once the whole piece is
                                     written */
}

/**
 * @brief  Can @p shape, rotated to @p rot, occupy the cells implied by pivot
 *         (@p px, @p py) without leaving the well or overlapping settled
 * blocks?
 *
 * @note   pivot.y always names the row the piece is about to move into, so a
 *         "can I fall?" test is just canPlace() at the unmodified pivot.
 *         TetrisMap never contains the falling piece itself (addFigure() is
 *         always paired with clearFigure()), so no self-collision is possible.
 */
static bool canPlace(const Shape_t *shape, uint8_t rot, int8_t px, int8_t py) {
  for (uint8_t i = 0; i < shape->len; i++) {
    for (uint8_t j = 0; j < shape->len; j++) {
      const Point_Value_t *p = &shape->shape[rot][i][j];
      if (p->value != 1)
        continue;

      int8_t x = p->x + px;
      int8_t y = p->y + py;

      if (x < 0 || x > 7 || y < 0 || y > 31)
        return false;
      if (TetrisMap[x][y] == 1)
        return false;
    }
  }
  return true;
}

bool canSpawn(Shape_t *shape) {
  return canPlace(shape, shape->rotateNum, shape->pivot.x, shape->pivot.y);
}

bool canGoDown(Shape_t *shape) {
  return canPlace(shape, shape->rotateNum, shape->pivot.x, shape->pivot.y);
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
void removeFullRows(void) {
  for (uint8_t i = 0; i < MAPHIGHT;) {
    uint8_t countTheRow = 0;
    for (uint8_t j = 0; j < MAPWIDTH; j++) {
      if (TetrisMap[j][i] == 1)
        countTheRow++;
    }
    if (countTheRow == MAPWIDTH) {
      userScore += 11000;

      /* Collapse everything above row i down by one. The shift
       * overwrites row i, so it needs no separate clear. */
      for (uint8_t k = i + 1; k < MAPHIGHT; k++) {
        for (uint8_t j = 0; j < MAPWIDTH; j++) {
          TetrisMap[j][k - 1] = TetrisMap[j][k];
        }
      }
      /* Nothing shifted into row 31: without this it keeps its old
       * contents and the top row is duplicated into row 30. */
      for (uint8_t j = 0; j < MAPWIDTH; j++) {
        TetrisMap[j][31] = 0;
      }
      continue; /* re-test row i, it holds new contents now */
    }
    i++;
  }
}

void clearFigure(Shape_t *shape) {
  for (uint8_t i = 0; i < shape->len; i++) {
    for (uint8_t j = 0; j < shape->len; j++) {
      if (shape->shape[shape->rotateNum][i][j].value == 1)
        TetrisMap[shape->pivot.x + shape->shape[shape->rotateNum][i][j].x]
                 [shape->pivot.y + shape->shape[shape->rotateNum][i][j].y] = 0;
    }
  }
}
void clearTetrisMap(void) { memset(TetrisMap, 0, sizeof(TetrisMap)); }
void moveFigureDown(Shape_t *shape) { shape->pivot.y -= 1; }

void convertTetrisMapToDisplayMap(void) {
  MAX7219_ClearDisplayMap();
  for (uint8_t i = 0; i < MAPHIGHT; i++) {
    uint8_t displayNum = (31 - i) / MAPWIDTH;
    for (uint8_t j = 0; j < 8; j++) {
      Display_Map[displayNum][7 - j] |= (TetrisMap[j][i] << (i % MAPWIDTH));
    }
  }
  MAX7219_UpdateDisplay();
}
