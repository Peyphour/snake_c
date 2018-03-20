//
// Created by bertrand on 20/03/18.
//

#ifndef UNTITLED_TYPES_H
#define UNTITLED_TYPES_H

#include "lib/graphics.h"
#include "lib/list.h"

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} DIRECTION;

typedef struct {
    list *snake;
    POINT *apple;
} game_state;

#endif //UNTITLED_TYPES_H
