#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <process.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define true 1
#define false 0
#define boolean _Bool//参照stdbool.h
#define _X 0
#define _Y 1
#define MAX_LEN map_height * map_width / 2
#define _NONE " "
#define _FOOD "●"
#define _BODY "■"
#define _HEAD_UP "▲"
#define _HEAD_DOWN "▼"
#define _HEAD_LEFT "■"
#define _HEAD_RIGHT "■"
#define COLOR_WHITE 0x0F
#define COLOR_GREEN 0x0A
#define KEY_SPACE 32

//#define _TEST "█▇■"

typedef struct node {
    int position[2];
    struct node *next;
} snake;

typedef enum {
    up,
    down,
    left,
    right
} direction;

void onEnter();            // 游戏开始前的处理
//void gameMenu();            // 游戏菜单,可能会加个自动play
void drawGameBorder();        // 绘制游戏边界
void printInfo();                    // 显示提示信息
DWORD WINAPI getInputFromKeyboard(LPVOID param);

void refreshSnake(snake *head, snake *tail, int food_position[2], direction direct);//refresh screen

boolean not_dead(snake *head, direction direct);//boolean

boolean is_eating(snake *head, direction direct, int **food_position);//boolean

void initHead(snake **head);

void moveBody(snake **head, direction direct, int **food_position);

void createFood(snake *head, int **food_position);

int *directionToPosition(int position[2], direction direct);

void printAtXY(int x, int y, unsigned color, char *ch);

void printLength();

int map_width = 50;
//X
int map_height = 30;
//Y
int length = 1;
direction direct;
boolean moved = true;
boolean playing = false;
boolean pause = false;
boolean notify = false;
long speed = 300;
HANDLE cursor;

/**
 * 可以，在Windows下面，用CreateThread(
 * LPSECURITY_ATTRIBUTES   lpThreadAttributes,  //1
 * DWORD   dwStackSize,                         //2
 * LPTHREAD_START_ROUTINE   lpStartAddress,     //3
 * LPVOID   lpParameter,                        //4
 * DWORD   dwCreateionFlags,                    //5
 * LPDWORD   lpThreadId)                        //6
 * 函数可以创建一个线程
 * 第一个参数指线程的安全属性的设定，
 * 第二个参数表示线程堆栈的大小，
 * 第三个参数表示线程函数名称，
 * 第四个参数线程执行的参数，
 * 第五个参数指线程的优先级，
 * 最后一个参数指向线程的ID。
 */

int main() {
    int *food_position;
    char last_input;
    LPVOID param = &last_input;
    snake *head = NULL;
    srand((unsigned) time(NULL));
    onEnter();
    while (true) {//游戏循环//假死循环，在子线程里有退出
        playing = true;
        HANDLE thread = CreateThread(NULL, 0, getInputFromKeyboard, param, THREAD_PRIORITY_NORMAL, NULL);
        drawGameBorder();
        initHead(&head);
        createFood(head, &food_position);
        direct = right;
        printInfo();
        while (not_dead(head, direct) && (last_input != 27 || last_input != KEY_SPACE)) {//游戏中循环
            if (pause) {
                if (notify) {
                    printAtXY(map_width + 1, map_height - 3, COLOR_GREEN, "按任意键继续游戏");
                } else {
                    printAtXY(map_width + 1, map_height - 3, COLOR_GREEN, "               ");
                }
            } else {
                moveBody(&head, direct, &food_position);
                printLength();
                moved = true;
                pause = false;
            }
            notify = !notify;
            Sleep((DWORD) speed);
        }
        notify = false;
        playing = false;
        CloseHandle(thread);
        fflush(stdin);
        char tmp_input = last_input;
        while (1) {
            if (tmp_input != last_input) {
                break;
            } else {
                if (notify) {
                    printAtXY(3, 3, COLOR_WHITE, "Game Over!");
                    printAtXY(3, 4, COLOR_WHITE, "Press E/e to exit,press the other key to replay");
                } else {
                    printAtXY(3, 3, COLOR_WHITE, "          ");
                    printAtXY(3, 4, COLOR_WHITE, "                                                ");
                }
                notify = !notify;
                Sleep(300);
            }
        }
        notify = false;
        free(&tmp_input);
        printAtXY(3, 3, COLOR_WHITE, "          ");
        printAtXY(3, 4, COLOR_WHITE, "                                                ");
        printAtXY(food_position[_X], food_position[_Y], COLOR_WHITE, "  ");
        snake *tmp;
        while (head != NULL) {
            tmp = head;
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, "  ");
            head = tmp->next;
            free(tmp);
        }
        length = 1;
        fflush(stdin);
    }
    return 0;
}

void initHead(snake **head) {
    *head = (snake *) malloc(sizeof(snake));
    (*head)->position[_X] = map_width / 4;
    (*head)->position[_Y] = map_height / 4 * 3;
    (*head)->next = NULL;
}

void moveBody(snake **head, direction direct, int **food_position) {
    int position[2];
    position[_X] = (*head)->position[_X];
    position[_Y] = (*head)->position[_Y];
    snake *tmp = *head;
    snake *tail = NULL;
    int *last_food_position = *food_position;
    if (is_eating(*head, direct, food_position)) {
        *head = (snake *) malloc(sizeof(snake));
        (*head)->position[_X] = last_food_position[_X];
        (*head)->position[_Y] = last_food_position[_Y];
        (*head)->next = tmp;
    } else {
        *head = (snake *) malloc(sizeof(snake));//新建头
        int *new_position = directionToPosition(position, direct);
        (*head)->position[_X] = new_position[_X];
        (*head)->position[_Y] = new_position[_Y];
        (*head)->next = tmp;
        if (tmp->next != NULL) {
            while (tmp->next->next != NULL) {
                tmp = tmp->next;
            }
            tail = tmp->next;
            tmp->next = NULL;
        } else {
            (*head)->next = NULL;
            tail = tmp;
        }
    }
    refreshSnake(*head, tail, *food_position, direct);//重绘蛇和食物
}

void createFood(snake *head, int **food_position) {
    *food_position = (int *) malloc(2 * sizeof(int));
    int repeat = 0;
    while (true) {
        snake *tmp = head;
        (*food_position)[_X] = (unsigned) rand() % (map_width - 2) + 1;
        (*food_position)[_Y] = (unsigned) rand() % (map_height - 2) + 1;
        while (tmp->next != NULL) {
            if ((*food_position)[_X] == tmp->position[_X] && (*food_position)[_Y] == tmp->position[_Y]) {
                repeat = 1;
            }
            tmp = tmp->next;
        }
        if (repeat == 0) {
            break;
        }
        repeat = 0;
    }
}

boolean not_dead(snake *head, direction direct) {
    int position[2];
    position[_X] = head->position[_X];
    position[_Y] = head->position[_Y];
    int *next_position = directionToPosition(position, direct);
    if (next_position[_X] < 0 || next_position[_X] > map_width - 2 || next_position[_Y] < 0 ||
        next_position[_Y] > map_height - 2) {//边界
        return false;
    } else {
        snake *tmp = head;
        while (tmp->next != NULL) {
            tmp = tmp->next;
            if (tmp->position[_X] == next_position[_X] && tmp->position[_Y] == next_position[_Y]) {//自己
                return false;
            }
        }
        return true;
    }
}

boolean is_eating(snake *head, direction direct, int **food_position) {
    int position[2];
    position[_X] = head->position[_X];
    position[_Y] = head->position[_Y];
    int *next_position = directionToPosition(position, direct);
    if (next_position[_X] == (*food_position)[_X] && next_position[_Y] == (*food_position)[_Y]) {
        createFood(head, food_position);
        if (length < MAX_LEN) {//当长度达到地图的一半（其实不止一半）时，长度不再增加,其实是为了减小难度，在想后面加障碍不
            length++;
            return true;
        }
        return false;
    } else {
        return false;
    }
}

void refreshSnake(snake *head, snake *tail, int *food_position, direction direct) {
    //绘制蛇和食物
    snake *tmp = head->next;
    switch (direct) {
        case up:
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, _HEAD_UP);
            break;
        case down:
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, _HEAD_DOWN);
            break;
        case left:
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, _HEAD_LEFT);
            break;
        case right:
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, _HEAD_RIGHT);
            break;
    }
    while (tmp != NULL && tmp != tail) {
        printAtXY(tmp->position[_X], tmp->position[_Y], COLOR_WHITE, _BODY);
        tmp = tmp->next;
    }
    if (tail != NULL) {
        printAtXY(tail->position[_X], tail->position[_Y], COLOR_WHITE, _NONE);
        free(tail);//free尾
    }
    printAtXY(food_position[_X], food_position[_Y], COLOR_WHITE, _FOOD);
}

int *directionToPosition(int position[2], direction direct) {
    switch (direct) {
        case up:
            position[_Y]--;
            break;
        case down:
            position[_Y]++;
            break;
        case left:
            position[_X]--;
            break;
        case right:
            position[_X]++;
            break;
    }
    return position;
}

void printAtXY(int x, int y, unsigned color, char *ch) {
    COORD pos;
    pos.X = (SHORT) (x << 1);
    pos.Y = (SHORT) y;
    // 移动到目标
    SetConsoleTextAttribute(cursor, (WORD) color);
    // 设置颜色
    SetConsoleCursorPosition(cursor, pos);
    printf("%s", ch);
}

void drawGameBorder() {
    int i, j;
    for (i = 0; i < map_height; i++) {//列
        for (j = 0; j < map_width; j++) {//行
            if (((i == 0 || i == map_height - 1) && i <= map_width) || (j == 0) ||
                (j == map_width - 1)) {//可以分开画成不同的边界，为了省事，直接统一吧。。。
                printAtXY(j, i, COLOR_WHITE, _BODY);
            }
        }
    }
    printInfo();
}

void onEnter() {
    cursor = GetStdHandle(STD_OUTPUT_HANDLE);        // 获取标准输出句柄
    CONSOLE_CURSOR_INFO cursorInfo = {1, false};            // 光标信息
    SetConsoleCursorInfo(cursor, &cursorInfo);    // 隐藏光标
    system("title VSnake");        // 设置控制台窗口标题
}

DWORD WINAPI getInputFromKeyboard(LPVOID param) {
    char *last_input = param;
    while (true) {
        char input = (char) _getch();
        if (!playing) {
            if (input == 'e' || input == 'E') {
                exit(0);
            } else {
                *last_input = input;
            }
        }
        if (input == KEY_SPACE) {
            if (*last_input == KEY_SPACE) {
                pause = false;
            } else {
                pause = true;
            }
        }
        *last_input = input;
        if (moved && !pause) {
            switch (input) {
                case 38:
                case 'w':    // 上
                    if (direct != down) {
                        direct = up;
                    }
                    break;
                case 40:
                case 's': //下
                    if (direct != up) {
                        direct = down;
                    }
                    break;
                case 37:
                case 'a': // 左
                    if (direct != right) {
                        direct = left;
                    }
                    break;
                case 39:
                case 'd':  // 右
                    if (direct != left) {
                        direct = right;
                    }
                    break;
                default:
                    break;
            }
            moved = false;
        }
    }
    return 0;
}


void printLength() {
    printAtXY(map_width + 1, 2, COLOR_GREEN, "Snake length:");
    printf("%d", length);
}

void printInfo() {
    printAtXY(map_width + 1, 4, COLOR_GREEN, "游戏提示:");
    printAtXY(map_width + 1, 5, COLOR_GREEN, "w,s,a,d操作");
    printAtXY(map_width + 1, 6, COLOR_GREEN, "空格,esc暂停");
}

/*void autoPlay() {

}*/

#pragma clang diagnostic pop