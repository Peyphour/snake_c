#include <X11/Xlib.h>
#include "types.h"
#include "constants.h"
#include <pthread.h>
#include <semaphore.h>
#include "astar.h"

#define DEBUG 0

DIRECTION currentDirection;
int running, drawStart, autoRunning;
POINT apple;
game_state state;
list snake, obstacles;
sem_t mutex;

int random_range(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void new_apple() {

    POINT p2;
    do {
        apple.y = random_range(2, STEP - 2) * STEP_H;
        apple.x = random_range(2, STEP - 2) * STEP_W;

        p2.x = apple.x + STEP_W;
        p2.y = apple.y - STEP_H;
    } while (list_count_occurences(&snake, &apple) > 1);

    POINT start = {0, 0};
    POINT end = {WIDTH, HEIGHT};
    draw_fill_rectangle(start, end, black);


    draw_fill_rectangle(apple, p2, vert);
}

bool print_list(void *data) {
#if DEBUG == 1
    printf("POINT(%d, %d) ", ((POINT *) data)->x, ((POINT *) data)->y);
    return (bool) TRUE;
#endif
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

        sem_wait(&mutex);

        drawStart = 1;
        list_for_each(&snake, recoverSnakeInBlack);
        list_prepend(&snake, calculateNextGeneration(snake.head->data));

        list_for_each(&snake, print_list);
        list_for_each(&obstacles, print_list);

        POINT *head = snake.head->data;
        if (head->x == apple.x && head->y == apple.y) {
            new_apple();
        } else {
            list_delete_last(&snake);
        }

        list_for_each(&snake, drawList);
        list_for_each(&obstacles, drawList);

        if (list_has_duplicate(&snake) || list_count_occurences(&obstacles, snake.head))
            running = 0;
        affiche_all();

        sem_post(&mutex);

        usleep(CLOCK);
    }
    return NULL;
}

static int mapAt(int x, int y) {
    if(x >= 0 && x < W_WIDTH && y >= 0 && y < W_HEIGHT) {
        POINT tmp = {x, y};
#if DEBUG == 1
        POINT tmp1 = {tmp.x + STEP_W, tmp.y - STEP_H};
        draw_fill_rectangle(tmp, tmp1, rouge);
#endif
        if(list_count_occurences(&snake, &tmp) == 0 && list_count_occurences(&obstacles, &tmp) == 0) { // No snake or obstacles in here
            return 0;
        } else {
            return 1;
        }
    } else
        return -1;
}

static void pathNodeNeighbours(ASNeighborList neighborList, void *node, void *context) {
    POINT *pathNode = node;

    if (mapAt(pathNode->x+STEP_W, pathNode->y) == 0) {
        POINT tmp = {pathNode->x + STEP_W, pathNode->y};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x-STEP_W, pathNode->y) == 0) {
        POINT tmp = {pathNode->x- STEP_W, pathNode->y};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x, pathNode->y+STEP_H) == 0) {
        POINT tmp = {pathNode->x, pathNode->y+STEP_H};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x, pathNode->y-STEP_H) == 0) {
        POINT tmp = {pathNode->x, pathNode->y-STEP_H};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
}

static float pathNodeHeuristic(void *fromNode, void *toNode, void *context) {
    POINT *from = fromNode;
    POINT *to = toNode;

    return (float) (fabs(from->x - to->x) * STEP_W + fabs(from->y - to->y) * STEP_H);
}

POINT get_auto_arrow() {

    sem_wait(&mutex);

    ASPathNodeSource source = {
            sizeof(POINT),
            &pathNodeNeighbours,
            &pathNodeHeuristic,
            NULL,
            NULL
    };

    ASPath path = ASPathCreate(&source, NULL, snake.head->data, &apple);

    if(ASPathGetCount(path) >= 1) {
        for(int i = 0; i < ASPathGetCount(path); i++) {
            POINT *node = ASPathGetNode(path, i);
#if DEBUG == 1
            POINT drawRec = {node->x + STEP_W , node->y - STEP_H };
            draw_fill_rectangle(*node, drawRec, blanc);
#endif
        }
        POINT *firstDirection = ASPathGetNode(path, 1);
        POINT direction = {
                (firstDirection->x - ((POINT*) snake.head->data)->x),
                (firstDirection->y - ((POINT*) snake.head->data)->y)
        };
        if((abs(direction.x + direction.y) != STEP_W && abs(direction.x + direction.y) != STEP_H) || list_count_occurences(&snake, firstDirection) > 0) {
            direction.x = 0; direction.y = 0;
        }
#if DEBUG == 1
        printf("direction(%d, %d) ", direction.x, direction.y);
        printf("head(%d, %d) firstDirection(%d, %d)\n", ((POINT*) snake.head->data)->x, ((POINT*) snake.head->data)->y, firstDirection->x, firstDirection->y);
#endif
        ASPathDestroy(path);
        sem_post(&mutex);
        return direction;
    } else {
        printf("no path\n");
    }
    ASPathDestroy(path);
    POINT defaultDirection = {0, 0};
    sem_post(&mutex);
    return defaultDirection;
}


static void *processKeyboard(void *data) {

    while (running) {

        POINT p;

        if (autoRunning)
            p = get_auto_arrow();
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

void initSnake(int size, DIRECTION initialDirection, int initX, int initY) {
    switch (initialDirection) {
        case LEFT:
            for (int i = 0; i < size; i++) {
                POINT p = {(initX + i) * STEP_W, initY * STEP_H};
                list_append(&snake, &p);
            }
            break;
        case RIGHT:
            for (int i = 0; i < size; i++) {
                POINT p = {(initX - i) * STEP_W, initY * STEP_H};
                list_append(&snake, &p);
            }
            break;
        case UP:
            for (int i = 0; i < size; i++) {
                POINT p = {initX * STEP_W, (initY - i) * STEP_H};
                list_append(&snake, &p);
            }
            break;
        case DOWN:
            for (int i = 0; i < size; i++) {
                POINT p = {initX * STEP_W, (initY + i) * STEP_H};
                list_append(&snake, &p);
            }
            break;
    }

    currentDirection = initialDirection;
}

void initObstacles() {
    POINT a = {W_WIDTH / 2, W_HEIGHT / 2};
    POINT b = {a.x + STEP_W, a.y + STEP_H};
    POINT c = {a.x - STEP_W, a.y + STEP_H};
    POINT d = {a.x + STEP_W, a.y - STEP_H};
    POINT e = {a.x - STEP_W, a.y - STEP_H};
    list_append(&obstacles, &a);
    list_append(&obstacles, &b);
    list_append(&obstacles, &c);
    list_append(&obstacles, &d);
    list_append(&obstacles, &e);
}

int main(int argc, char *argv[]) {

    XInitThreads();
    init_graphics(W_WIDTH, W_HEIGHT, "SNAKE");

    if (argc > 1) {
        if (!strcmp(argv[1], "auto"))
            autoRunning = 1;
    }

    running = 1;

    affiche_auto_off();

    list_new(&snake, sizeof(POINT), NULL);
    list_new(&obstacles, sizeof(POINT), NULL);

    srand((unsigned int) time(NULL));

    initSnake(STEP / 10, LEFT, STEP / 4, STEP / 2);
    initObstacles();

    new_apple();

    state.apple = &apple;
    state.snake = &snake;

    sem_init(&mutex, 0, 1);

    pthread_t drawThread, keyboardThread;

    pthread_create(&drawThread, NULL, draw, NULL);
    pthread_create(&keyboardThread, NULL, processKeyboard, NULL);

    pthread_join(drawThread, NULL);
    pthread_join(keyboardThread, NULL);
    sem_destroy(&mutex);
    list_destroy(&snake);
    list_destroy(&obstacles);

    wait_escape();

    return 0;
}
