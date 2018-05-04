#include <X11/Xlib.h>
#include "types.h"
#include "constants.h"
#include <pthread.h>
#include "astar.h"

DIRECTION currentDirection;
int running, drawStart, autoRunning;
POINT apple;
game_state state;
list snake;

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
        list_for_each(&snake, recoverSnakeInBlack);
        list_prepend(&snake, calculateNextGeneration(snake.head->data));

        POINT *head = snake.head->data;
        if (head->x == apple.x && head->y == apple.y) {
            new_apple();
        } else {
            list_delete_last(&snake);
        }

        list_for_each(&snake, drawList);

        if (list_has_duplicate(&snake))
            running = 0;
        affiche_all();

        usleep(CLOCK);
    }
    return NULL;
}

static int mapAt(int x, int y) {
    //printf("mapAt(%d, %d)\n", (x % STEP) * STEP_W, (y % STEP) * STEP_H);
    if(x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        POINT tmp = {(x % STEP) * STEP_W, (y % STEP) * STEP_H};
        if(list_count_occurences(&snake, &tmp) == 0) { // No snake in here
            return 0;
        } else {
            return 1;
        }
    } else
        return -1;
}

static void pathNodeNeighours(ASNeighborList neighborList, void *node, void *context) {
    POINT *pathNode = node;

    if (mapAt(pathNode->x+1, pathNode->y) == 0) {
        POINT tmp = {pathNode->x+ STEP_W, pathNode->y};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x-1, pathNode->y) == 0) {
        POINT tmp = {pathNode->x- STEP_W, pathNode->y};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x, pathNode->y+1) == 0) {
        POINT tmp = {pathNode->x, pathNode->y+STEP_H};
        ASNeighborListAdd(neighborList, &tmp, 1);
    }
    if (mapAt(pathNode->x, pathNode->y-1) == 0) {
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

    ASPathNodeSource source = {
            sizeof(POINT),
            &pathNodeNeighours,
            &pathNodeHeuristic,
            NULL,
            NULL
    };

    ASPath path = ASPathCreate(&source, NULL, snake.head->data, &apple);

    if(ASPathGetCount(path) >= 1) {
        for(int i = 0; i < ASPathGetCount(path); i++) {
            POINT *node = ASPathGetNode(path, i);
            POINT drawRec = {node->x + STEP_W / 2, node->y - STEP_H / 2};
            draw_fill_rectangle(*node, drawRec, blanc);
        }
        POINT *firstDirection = ASPathGetNode(path, 1);
        POINT direction = {
                (firstDirection->x - ((POINT*) snake.head->data)->x),
                (firstDirection->y - ((POINT*) snake.head->data)->y)
        };
        printf("direction(%d, %d) ", direction.x, direction.y);
        printf("head(%d, %d) firstDirection(%d, %d)\n", ((POINT*) snake.head->data)->x, ((POINT*) snake.head->data)->y, firstDirection->x, firstDirection->y);

        return direction;
    } else {
        printf("no path\n");
    }
    POINT defaultDirection = {0, 0};
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

int main(int argc, char *argv[]) {

    XInitThreads();
    init_graphics(W_WIDTH, W_HEIGHT, "test");

    if (argc > 1) {
        if (!strcmp(argv[1], "auto"))
            autoRunning = 1;
    }

    running = 1;

    affiche_auto_off();

    list_new(&snake, sizeof(POINT), NULL);

    srand((unsigned int) time(NULL));

    initSnake(STEP / 10, LEFT, STEP / 2, STEP / 2);

    new_apple();

    state.apple = &apple;
    state.snake = &snake;

    pthread_t drawThread, keyboardThread;

    pthread_create(&drawThread, NULL, draw, NULL);
    pthread_create(&keyboardThread, NULL, processKeyboard, NULL);

    pthread_join(drawThread, NULL);
    pthread_join(keyboardThread, NULL);

    wait_escape();

    return 0;
}
