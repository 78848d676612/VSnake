#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <process.h>

#define true 1
#define false 0
#define boolean _Bool//����stdbool.h
#define _X 0
#define _Y 1
#define MAX_LEN map_height * map_width / 2
#define _NONE " "
#define _FOOD "��"
#define _BODY "��"
#define _HEAD_UP "��"
#define _HEAD_DOWN "��"
#define _HEAD_LEFT "��"
#define _HEAD_RIGHT "��"
#define _TEST "���~��"

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

void onEnter();            // ��Ϸ��ʼǰ�Ĵ���
void gameMenu();            // ��Ϸ�˵�,���ܻ�Ӹ��Զ�play

void drawGameBorder();        // ������Ϸ�߽�
void printInfo();                    // ��ʾ��ʾ��Ϣ
DWORD WINAPI getInputFromKeyboard(LPVOID param);

void refreshSnake(snake *head, snake *tail, int food_position[2], direction direct);//refresh screen

boolean not_dead(snake *head, direction direct);//boolean

boolean is_eating(snake *head, direction direct, int **food_position);//boolean//in move

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
unsigned int color = 0x0F;
boolean moved = true;
boolean playing = false;
long speed = 300;
HANDLE handle;
FNCALLBACK onKeyDown;//�ص�

/**
 * ���ԣ���Windows���棬��CreateThread(
 * LPSECURITY_ATTRIBUTES   lpThreadAttributes,  //1
 * DWORD   dwStackSize,                         //2
 * LPTHREAD_START_ROUTINE   lpStartAddress,     //3
 * LPVOID   lpParameter,                        //4
 * DWORD   dwCreateionFlags,                    //5
 * LPDWORD   lpThreadId)                        //6
 * �������Դ���һ���߳�
 * ��һ������ָ�̵߳İ�ȫ���Ե��趨���ڶ���������ʾ�̶߳�ջ�Ĵ�С��������������ʾ�̺߳������ƣ����ĸ������߳�ִ�еĲ��������������ָ�̵߳����ȼ������һ������ָ���̵߳�ID��
 * ����windows����C�������߳̿��Բ��MSDN���ɡ� ������ǲ����Ļ�������¥����Ū�����������ϵͳ��ԭ������Ū��������߳�����̵���ͬ���ϵ
 *
 *
 */

int main() {
    int *food_position;
    char last_input;
    LPVOID param = &last_input;
    snake *head = NULL;
    srand((unsigned) time(NULL));
    onEnter();
    while (1) {//��Ϸѭ��
        playing = true;
        HANDLE thread = CreateThread(NULL, 0, getInputFromKeyboard, param, THREAD_PRIORITY_NORMAL, NULL);
        drawGameBorder();
        initHead(&head);
        createFood(head, &food_position);
        direct = right;
        while (not_dead(head, direct)) {//��Ϸ��ѭ��
            moveBody(&head, direct, &food_position);
            printLength();
            moved = true;
            Sleep((DWORD) speed);
        }
        playing = false;
        printAtXY(3, 3, 0x0f, "Game Over!");
        printAtXY(3, 4, 0x0f, "Press E/e to exit,press the other key to replay");
        CloseHandle(thread);
        fflush(stdin);
        char key = (char) _getch();
        if (key == 'e' || key == 'E') {
            break;
        }
        printAtXY(3, 3, 0x0f, "          ");
        printAtXY(3, 4, 0x0f, "                                                ");
        printAtXY(food_position[_X], food_position[_Y], 0x0f, "  ");
        snake *tmp;
        while (head != NULL) {
            tmp = head;
            printAtXY(head->position[_X], head->position[_Y], 0x0f, "  ");
            head = tmp->next;
            free(tmp);
        }
        length = 1;
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
        *head = (snake *) malloc(sizeof(snake));//�½�ͷ
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
    refreshSnake(*head, tail, *food_position, direct);//�ػ��ߺ�ʳ��
}

void createFood(snake *head, int **food_position) {
    *food_position = (int *) malloc(2 * sizeof(int));
    int repeat = 0;
    while (1) {
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
//            printf("food_position------>x:%d,y:%d\n", food_position[_X], food_position[_Y]);
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
        next_position[_Y] > map_height - 2) {//�߽�
        return false;
    } else {
        snake *tmp = head;
        while (tmp->next != NULL) {
            tmp = tmp->next;
            if (tmp->position[_X] == next_position[_X] && tmp->position[_Y] == next_position[_Y]) {//�Լ�
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
        if (length < MAX_LEN) {//�����ȴﵽ��ͼ��һ�루��ʵ��ֹһ�룩ʱ�����Ȳ�������
            length++;
            return true;
        }
        return false;
    } else {
        return false;
    }
}

void refreshSnake(snake *head, snake *tail, int *food_position, direction direct) {
    //�����ߺ�ʳ��
    snake *tmp = head->next;
    switch (direct) {
        case up:
            printAtXY(head->position[_X], head->position[_Y], color, _HEAD_UP);
            break;
        case down:
            printAtXY(head->position[_X], head->position[_Y], color, _HEAD_DOWN);
            break;
        case left:
            printAtXY(head->position[_X], head->position[_Y], color, _HEAD_LEFT);
            break;
        case right:
            printAtXY(head->position[_X], head->position[_Y], color, _HEAD_RIGHT);
            break;
    }
    while (tmp != NULL && tmp != tail) {
        printAtXY(tmp->position[_X], tmp->position[_Y], color, _BODY);
        tmp = tmp->next;
    }
    if (tail != NULL) {
        printAtXY(tail->position[_X], tail->position[_Y], color, _NONE);
        free(tail);//freeβ
    }
    printAtXY(food_position[_X], food_position[_Y], color, _FOOD);
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
    // �ƶ���Ŀ��
    SetConsoleTextAttribute(handle, (WORD) color);
    // ������ɫ
    SetConsoleCursorPosition(handle, pos);
    printf("%s", ch);
}

void drawGameBorder() {
    int i, j;
    for (i = 0; i < map_height; i++) {//��
        for (j = 0; j < map_width; j++) {//��
            if (((i == 0 || i == map_height - 1) && i <= map_width) || (j == 0) ||
                (j == map_width - 1)) {//���Էֿ����ɲ�ͬ�ı߽磬Ϊ��ʡ�£�ֱ��ͳһ�ɡ�����
                printAtXY(j, i, color, _BODY);
            }
        }
    }
    printAtXY(map_width + 1, map_height - 4, 0x0A, "w,s,a,d����");
    printAtXY(map_width + 1, map_height - 3, 0x0A, "�ո�,esc��ͣ");
}

void onEnter() {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);        // ��ȡ��׼������
    CONSOLE_CURSOR_INFO cursorInfo = {1, false};            // �����Ϣ
    SetConsoleCursorInfo(handle, &cursorInfo);    // ���ع��
    system("title VSnake");        // ���ÿ���̨���ڱ���
}

DWORD WINAPI getInputFromKeyboard(LPVOID param) {
    char *last_input = param;
    while (playing) {
        char input = (char) _getch();
        *last_input = input;
        // ��ͣ
        if (input == 27 || input == 32) {
            printAtXY(map_width + 1, 8, 0x0A, "�������������Ϸ");
            system("pause 1>nul");
            printAtXY(map_width + 1, 3, 0x0A, "               ");
            fflush(stdin);
        }
        if (moved) {
            switch (input) {
                case 38:
                case 'w':    // ��
                    if (direct != down) {
                        direct = up;
                    }
                    break;
                case 40:
                case 's': //��
                    if (direct != up) {
                        direct = down;
                    }
                    break;
                case 37:
                case 'a': // ��
                    if (direct != right) {
                        direct = left;
                    }
                    break;
                case 39:
                case 'd':  // ��
                    if (direct != left) {
                        direct = right;
                    }
                    break;
                default:break;
            }
            moved = false;
        }
    }
    return 0;
}


void printLength() {
    printAtXY(map_width + 1, 2, 0x0A, "Snake length:");
    printf("%d", length);
}


