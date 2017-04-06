#include "list.h"
#include "graphics.h"
#include <pthread.h>

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} DIRECTION;

#define W_WIDTH 1000
#define W_HEIGHT 1000
#define STEP 100

#define STEP_W W_WIDTH / STEP
#define STEP_H W_HEIGHT / STEP

DIRECTION currentDirection;
int running, drawStart;
POINT apple;

void new_apple() {
    apple.y = (rand() % STEP) * STEP_H;
    apple.x = (rand() % STEP) * STEP_W;

    printf("new apple at %d/%d", apple.x, apple.y);

    POINT p2 = {apple.x + STEP_W, apple.y - STEP_H};

    draw_fill_rectangle(apple, p2, vert);
}

bool drawList(void *data) {
    POINT p;

    p.y = ((POINT *) data)->y;
    p.x = ((POINT *) data)->x;

    POINT p2 = {p.x + STEP_W, p.y - STEP_H};

    if(drawStart) {
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
        if(head->x == apple.x && head->y == apple.y) {
            new_apple();
        } else {
            list_delete_last((list *) data);
        }

        list_for_each((list *) data, drawList);

        if (list_has_duplicate((list *) data))
            running = 0;
        affiche_all();

        usleep(20000);
    }
    return NULL;
}

static void *processKeyboard(void *data) {
    while (running) {
        POINT p = get_arrow();
        if (p.x < 0)
            currentDirection = LEFT;
        else if (p.x > 0)
            currentDirection = RIGHT;
        else if (p.y < 0)
            currentDirection = DOWN;
        else if (p.y > 0)
            currentDirection = UP;
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

int main() {

    init_graphics(W_WIDTH, W_HEIGHT, "test");

    running = 1;

    affiche_auto_off();

    list snake;
    list_new(&snake, sizeof(POINT), NULL);

    srand((unsigned int) time(NULL));

    initSnake(&snake, 15, UP, 50, 50);

    new_apple();

    pthread_t drawThread, keyboardThread;

    pthread_create(&drawThread, NULL, draw, &snake);
    pthread_create(&keyboardThread, NULL, processKeyboard, NULL);

    pthread_join(drawThread, NULL);
    pthread_join(keyboardThread, NULL);

    wait_escape();

    return 0;
}