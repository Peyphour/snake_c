//
// Created by bertrand on 20/03/18.
//

#ifndef UNTITLED_TYPES_H
#define UNTITLED_TYPES_H

#include "linked_list.h"
#include "graphics.h"

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} DIRECTION;

typedef struct {
    list *snake;
    POINT *apple;
    int score;
} game_state;

#endif //UNTITLED_TYPES_H
