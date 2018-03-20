#include "lib/list.h"
#include <X11/Xlib.h>
#include "lib/graphics.h"
#include "types.h"
#include "constants.h"
#include <pthread.h>

DIRECTION currentDirection;
int running, drawStart, autoRunning;
POINT apple;

bool obstacle_in_direction(list *snake, DIRECTION direction);

int random_range(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void new_apple(list *snake) {

    POINT p2;
    do {
        apple.y = random_range(2, STEP - 2) * STEP_H;
        apple.x = random_range(2, STEP - 2) * STEP_W;

        p2.x = apple.x + STEP_W;
        p2.y = apple.y - STEP_H;
    } while (list_count_occurences(snake, &apple) > 1);


    draw_fill_rectangle(apple, p2, vert);
}

bool drawList(void *data) {
    POINT p;

    p.y = ((POINT *) data)->y;
    p.x = ((POINT *) data)->x;


    POINT p2 = {p.x + STEP_W, p.y - STEP_H};

    if (drawStart) {
        draw_fill_rectangle(p, p2, rouge);
        drawStart = 0;
    } else
        draw_fill_rectangle(p, p2, jaune);

    return (bool) TRUE;
}

bool recoverSnakeInBlack(void *data) {

    POINT p;

    p.y = ((POINT *) data)->y;
    p.x = ((POINT *) data)->x;

    POINT p2 = {p.x + STEP_W, p.y - STEP_H};

    draw_fill_rectangle(p, p2, noir);

    return (bool) TRUE;
}

void *calculateNextGeneration(void *data) {

    POINT *p = malloc(sizeof(POINT));

    memcpy(p, data, sizeof(POINT));

    switch (currentDirection) {
        case LEFT:
            p->x -= STEP_W;
            break;
        case RIGHT:
            p->x += STEP_W;
            break;
        case UP:
            p->y += STEP_H;
            break;
        case DOWN:
            p->y -= STEP_H;
            break;
    }

    if (p->x < STEP_W || p->x >= (W_WIDTH - STEP_W) || p->y < STEP_H || p->y > W_HEIGHT) {
        running = false;
    }

    return p;
}

static void *draw(void *data) {
    while (running) {
        drawStart = 1;
        list_for_each((list *) data, recoverSnakeInBlack);
        list_prepend((list *) data, calculateNextGeneration(((list *) data)->head->data));

        POINT *head = ((list *) data)->head->data;
        if (head->x == apple.x && head->y == apple.y) {
            new_apple((list *) data);
        } else {
            list_delete_last((list *) data);
        }

        list_for_each((list *) data, drawList);

        if (list_has_duplicate((list *) data))
            running = 0;
        affiche_all();

        usleep(CLOCK);
    }
    return NULL;
}

bool obstacle_in_direction(list *snake, DIRECTION direction) {

    POINT *head = snake->head->data;

    switch (direction) {
        case UP:
            for (int i = head->y + STEP_H; i < W_HEIGHT; i += STEP_H) {
                POINT p = {head->x, i};
                if (list_count_occurences(snake, &p))
                    return (bool) TRUE;
            }
            break;
        case DOWN:
            for (int i = head->y - STEP_H; i > 0; i -= STEP_H) {
                POINT p = {head->x, i};
                if (list_count_occurences(snake, &p))
                    return (bool) TRUE;
            }
            break;
        case LEFT:
            for (int i = head->x + STEP_W; i < W_WIDTH; i += STEP_W) {
                POINT p = {i, head->y};
                if (list_count_occurences(snake, &p))
                    return (bool) TRUE;
            }
            break;
        case RIGHT:
            for (int i = head->x - STEP_W; i > 0; i -= STEP_W) {
                POINT p = {i, head->y};
                if (list_count_occurences(snake, &p))
                    return (bool) TRUE;
            }
            break;
    }

    return (bool) FALSE;
}

POINT get_first_direction_without_obstacle(list *snake) {

    POINT arrow = {0, 0};

    if (!obstacle_in_direction(snake, LEFT) && currentDirection != RIGHT) {
        arrow.x = -1;
    } else if (obstacle_in_direction(snake, LEFT) && !obstacle_in_direction(snake, DOWN) && currentDirection != UP) {
        arrow.y = -1;
    } else if (obstacle_in_direction(snake, LEFT) && obstacle_in_direction(snake, DOWN) &&
               !obstacle_in_direction(snake, RIGHT) && currentDirection != LEFT) {
        arrow.x = 1;
    } else if (obstacle_in_direction(snake, LEFT) && obstacle_in_direction(snake, DOWN) &&
               obstacle_in_direction(snake, RIGHT)
               && !obstacle_in_direction(snake, UP) && currentDirection != DOWN) {
        arrow.y = 1;
    }

    return arrow;
}


POINT get_auto_arrow(game_state *state) {

    POINT *head = state->snake->head->data;

    POINT arrow = {0, 0};

    if (head->x - apple.x < 0)
        arrow.x = 1;
    else if (head->x - apple.x > 0)
        arrow.x = -1;
    else if (head->y - apple.y < 0)
        arrow.y = 1;
    else if (head->y - apple.y > 0)
        arrow.y = -1;


    if (arrow.x > 0 && obstacle_in_direction(state->snake, LEFT) ||
        arrow.x < 0 && obstacle_in_direction(state->snake, RIGHT)
        || arrow.y > 0 && obstacle_in_direction(state->snake, UP) ||
        arrow.y < 0 && obstacle_in_direction(state->snake, DOWN)) {
        arrow = get_first_direction_without_obstacle(state->snake);
    }

    return arrow;
}


static void *processKeyboard(void *data) {

    game_state *state = (game_state *) data;

    while (running) {

        POINT p;

        if (autoRunning)
            p = get_auto_arrow(state);
        else
            p = get_arrow();

        if (p.x < 0)
            currentDirection = LEFT;
        else if (p.x > 0)
            currentDirection = RIGHT;
        else if (p.y < 0)
            currentDirection = DOWN;
        else if (p.y > 0)
            currentDirection = UP;

        usleep(CLOCK / 2);

    }
    return NULL;
}

void initSnake(list *snake, int size, DIRECTION initialDirection, int initX, int initY) {
    switch (initialDirection) {
        case LEFT:
            for (int i = 0; i < size; i++) {
                POINT p = {(initX + i) * STEP_W, initY * STEP_H};
                list_append(snake, &p);
            }
            break;
        case RIGHT:
            for (int i = 0; i < size; i++) {
                POINT p = {(initX - i) * STEP_W, initY * STEP_H};
                list_append(snake, &p);
            }
            break;
        case UP:
            for (int i = 0; i < size; i++) {
                POINT p = {initX * STEP_W, (initY - i) * STEP_H};
                list_append(snake, &p);
            }
            break;
        case DOWN:
            for (int i = 0; i < size; i++) {
                POINT p = {initX * STEP_W, (initY + i) * STEP_H};
                list_append(snake, &p);
            }
            break;
    }

    currentDirection = initialDirection;
}

int main(int argc, char *argv[]) {

    game_state state;

    XInitThreads();
    init_graphics(W_WIDTH, W_HEIGHT, "test");


    if (argc > 1) {
        if (!strcmp(argv[1], "auto"))
            autoRunning = 1;
    }

    running = 1;

    affiche_auto_off();

    list snake;
    list_new(&snake, sizeof(POINT), NULL);

    srand((unsigned int) time(NULL));

    initSnake(&snake, STEP / 10, UP, STEP / 2, STEP / 2);

    new_apple(&snake);

    state.apple = &apple;
    state.snake = &snake;

    pthread_t drawThread, keyboardThread;

    pthread_create(&drawThread, NULL, draw, &snake);
    pthread_create(&keyboardThread, NULL, processKeyboard, &state);

    pthread_join(drawThread, NULL);
    pthread_join(keyboardThread, NULL);

    wait_escape();

    return 0;
}
