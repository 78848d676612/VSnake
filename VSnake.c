#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "Base64.c"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define true 1
#define false 0
#define boolean _Bool//����stdbool.h
#define _X 0
#define _Y 1
#define MAX_LEN map_height * map_width / 2
#define _NONE "  "
#define _FOOD "��"
#define _BODY "��"
#define _HEAD_UP "��"
#define _HEAD_DOWN "��"
#define _HEAD_LEFT "<"//������������ʾ������= =,��������------->??
#define _HEAD_RIGHT ">"
#define COLOR_WHITE 0x0F
#define COLOR_GREEN 0x0A
#define KEY_SPACE 32

typedef struct node {
    int position[2];
    struct node *next;
} snake;

typedef enum { //Ϊ���Զ��㷨��д�㣬��ֵ�̶�
    up = 2,
    down = -2,
    left = -1,
    right = 1,
    none = 0
} direction;

void onEnter();

boolean gameMenu();

void drawGameBorder();

void printInfo();

void removeInfo();

void printMenu();

void removeMenu();

DWORD WINAPI getInputFromKeyboard(LPVOID param);

void refreshSnake(snake *head, snake *tail, int food_position[2], direction direct);//refresh screen

boolean not_dead(snake *head, direction direct);//boolean

boolean is_eating(snake *head, direction direct, int **food_position);//boolean

void initHead(snake **head);

void moveBody(snake **head, direction direct, int **food_position);

void createFood(snake *head, int **food_position);

int *directionToPosition(int position[2], direction direct);

void printAtXY(int x, int y, unsigned color, char *ch);

void printLengthAndScore();

void autoPlay(snake *head, int food_position[2]);

void inputToDirection(char input);

void storeMaxScore(int max_score);

int getMaxScore();

direction turn(snake *head, direction *_direction);

int map_width = 50;
int map_height = 30;
int length = 1;
int score = 0;
int max_score;
direction direct;
boolean moved = true;
boolean playing = false;
boolean pause = false;
boolean notify = false;
boolean auto_play = false;
long speed = 200;
HANDLE cursor;
HANDLE control_thread;

int main() {
    int *food_position;
    char last_input;
    LPVOID param = &last_input;
    snake *head = NULL;
    srand((unsigned) time(NULL));
    onEnter();
    auto_play = gameMenu();
    max_score = getMaxScore();
    drawGameBorder();
    while (true) {//��Ϸѭ��//����ѭ���������߳������˳�
        playing = true;
        control_thread = CreateThread(NULL, 0, getInputFromKeyboard, param, THREAD_PRIORITY_NORMAL, NULL);
        initHead(&head);
        createFood(head, &food_position);
        direct = right;
        printInfo();
        while (not_dead(head, direct) && (last_input != 27 || last_input != KEY_SPACE)) {//��Ϸ��ѭ��
            if (pause) {
                if (notify) {
                    printAtXY(map_width + 1, map_height - 3, COLOR_GREEN, "�������������Ϸ");
                } else {
                    printAtXY(map_width + 1, map_height - 3, COLOR_GREEN, "               ");
                }
            } else {
                moveBody(&head, direct, &food_position);
                if (auto_play) {
                    autoPlay(head, food_position);
                }
                printLengthAndScore();
                moved = true;
                pause = false;
            }
            notify = !notify;
            Sleep((DWORD) speed);
        }
        if (score > max_score) {
            storeMaxScore(score);
            max_score = score;
        }
        notify = false;
        playing = false;
        if (!auto_play) {
            CloseHandle(control_thread);
        }
        fflush(stdin);
        last_input = 0;//����ϴ�����
        removeInfo();
        while (1) {
            if (last_input == 'A' || last_input == 'a') {
                auto_play = true;
                break;
            } else if (last_input == 'S' || last_input == 's') {
                auto_play = false;
                break;
            } else {
                if (notify) {
                    printMenu();
                } else {
                    removeMenu();
                }
                notify = !notify;
                Sleep(500);
            }
        }
        notify = false;
        removeMenu();
        printAtXY(food_position[_X], food_position[_Y], COLOR_WHITE, _NONE);
        snake *tmp;
        while (head != NULL) {
            tmp = head;
            printAtXY(head->position[_X], head->position[_Y], COLOR_WHITE, _NONE);
            head = tmp->next;
            free(tmp);
        }
        length = 1;
        score = 0;
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
    if (next_position[_X] < 1 || next_position[_X] > map_width - 2 || next_position[_Y] < 1 ||
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
        if (length < MAX_LEN) {//�����ȴﵽ��ͼ��һ�루��ʵ��ֹһ�룩ʱ�����Ȳ�������,��ʵ��Ϊ�˼�С�Ѷȣ����������ϰ���
            length++;
        }
        score++;
        return true;
    } else {
        return false;
    }
}

void refreshSnake(snake *head, snake *tail, int *food_position, direction direct) {
    //�����ߺ�ʳ��
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
        default:
            break;
    }
    while (tmp != NULL && tmp != tail) {
        printAtXY(tmp->position[_X], tmp->position[_Y], COLOR_WHITE, _BODY);
        tmp = tmp->next;
    }
    if (tail != NULL) {
        printAtXY(tail->position[_X], tail->position[_Y], COLOR_WHITE, _NONE);
        free(tail);//freeβ
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
        default:
            break;
    }
    return position;
}

void printAtXY(int x, int y, unsigned color, char *ch) {
    COORD pos;
    pos.X = (SHORT) (x << 1);
    pos.Y = (SHORT) y;
    // �ƶ���Ŀ��
    SetConsoleTextAttribute(cursor, (WORD) color);
    // ������ɫ
    SetConsoleCursorPosition(cursor, pos);
    printf("%s", ch);
}

void drawGameBorder() {
    int i, j;
    for (i = 0; i < map_height; i++) {//��
        for (j = 0; j < map_width; j++) {//��
            if (((i == 0 || i == map_height - 1) && i <= map_width) || (j == 0) ||
                (j == map_width - 1)) {//���Էֿ����ɲ�ͬ�ı߽磬Ϊ��ʡ�£�ֱ��ͳһ�ɡ�����
                printAtXY(j, i, COLOR_WHITE, _BODY);
            }
        }
    }
}

void onEnter() {
    cursor = GetStdHandle(STD_OUTPUT_HANDLE);        // ��ȡ��׼������
    CONSOLE_CURSOR_INFO cursorInfo = {1, false};            // �����Ϣ
    SetConsoleCursorInfo(cursor, &cursorInfo);    // ���ع��
    system("title VSnake");        // ���ÿ���̨���ڱ���
}

DWORD WINAPI getInputFromKeyboard(LPVOID param) {
    char *last_input = param;
    while (true) {
        char input = (char) _getch();
        if (!playing) {
            if (input == 'e' || input == 'E') {
                exit(0);
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
        if (auto_play) {
            continue;
        }
        inputToDirection(input);
    }
    return 0;
}

void printLengthAndScore() {
    printAtXY(map_width + 1, 2, COLOR_GREEN, "Snake length:");
    printf("%d", length);
    printAtXY(map_width + 1, 3, COLOR_GREEN, "Score:");
    printf("%d", score);
    printAtXY(map_width + 1, 4, COLOR_GREEN, "Max Score:");
    printf("%d", max_score);
}

void printInfo() {
    if (auto_play) {
        printAtXY(map_width + 1, 6, COLOR_GREEN, "�Զ���");
    } else {
        printAtXY(map_width + 1, 6, COLOR_GREEN, "��Ϸ��ʾ:");
        printAtXY(map_width + 1, 7, COLOR_GREEN, "w,s,a,d����");
        printAtXY(map_width + 1, 8, COLOR_GREEN, "�ո�,esc��ͣ");
    }
}

void removeInfo() {
    if (auto_play) {
        printAtXY(map_width + 1, 6, COLOR_GREEN, "      ");
    } else {
        printAtXY(map_width + 1, 6, COLOR_GREEN, "         ");
        printAtXY(map_width + 1, 7, COLOR_GREEN, "           ");
        printAtXY(map_width + 1, 8, COLOR_GREEN, "            ");
    }
}

void printMenu() {
    printAtXY(3, 3, COLOR_WHITE, "Game Over!");
    printAtXY(3, 4, COLOR_WHITE, "E/e to exit");
    printAtXY(3, 5, COLOR_WHITE, "A/a to auto play");
    printAtXY(3, 6, COLOR_WHITE, "S/s to start play");
}

void removeMenu() {
    printAtXY(3, 3, COLOR_WHITE, "          ");
    printAtXY(3, 4, COLOR_WHITE, "           ");
    printAtXY(3, 5, COLOR_WHITE, "                ");
    printAtXY(3, 6, COLOR_WHITE, "                 ");
}

boolean gameMenu() {
    printAtXY(6, 4, COLOR_WHITE, "Snake");
    printAtXY(3, 5, COLOR_WHITE, "A/a to auto play");
    printAtXY(3, 6, COLOR_WHITE, "S/s to start play");
    char tmp = 0;
    while (tmp != 'a' && tmp != 's' && tmp != 'A' && tmp != 'S') {
        tmp = (char) _getch();
    }
    switch (tmp) {
        case 'a':
        case 'A':
            removeMenu();
            return true;
        case 's':
        case 'S':
            removeMenu();
            return false;
        default:
            break;
    }
    return false;
}

direction turn(snake *head, direction *_direction) {
    if (*_direction == none) {//�������ˣ�û�н������
        return direct;
    }
    if (*_direction != -direct && not_dead(head, *_direction)) {
        return *_direction;
    } else return turn(head, _direction + 1);
}

void autoPlay(snake *head, int food_position[2]) {//�����������ʳ���жϸ���ô�ߣ��߽�Ϊȫ�ֱ�����ֱ�Ӷ�ȡ//�Զ��㷨������Ū������������ֱ��if�жϣ�����˼·��ȫ���ˡ�����
    int horizontal = head->position[_X] - food_position[_X];
    int vertical = head->position[_Y] - food_position[_Y];
    if (horizontal == 0) {//һ��������
        if (vertical < 0) {//ͷ��ʳ����
            direction _direction[] = {down, right, left, none};
            direct = turn(head, _direction);
        } else {//ͷ��ʳ����
            direction _direction[] = {up, right, left, none};
            direct = turn(head, _direction);
        }
    } else if (horizontal > 0) {//ͷ��ʳ����
        if (vertical == 0) {//ͬһˮƽ��
            direction _direction[] = {left, up, down, none};
            direct = turn(head, _direction);
        } else if (vertical < 0) {//ͷ����ʳ������
            direction _direction[] = {left, down, up, right, none};
            direct = turn(head, _direction);
        } else {//ͷ����ʳ������
            direction _direction[] = {left, up, right, down, none};
            direct = turn(head, _direction);
        }
    } else {//ͷ��ʳ����
        if (vertical == 0) {//ͬһˮƽ��
            direction _direction[] = {right, down, up, none};
            direct = turn(head, _direction);
        } else if (vertical < 0) {//ͷ����ʳ������
            direction _direction[] = {right, down, left, up, none};
            direct = turn(head, _direction);
        } else {//ͷ����ʳ������
            direction _direction[] = {right, up, down, left, none};
            direct = turn(head, _direction);
        }
    }
}

void inputToDirection(char input) {
    if (moved && !pause) {
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
            default:
                break;
        }
        moved = false;
    }
}

void storeMaxScore(int score) {
    char *out, score_char[5];
    itoa(score, score_char, 10);
    out = base64_encode(score_char, sizeof(score_char));
    FILE *score_file = fopen("scores", "wb");
    fwrite(out, sizeof(out), 1, score_file);
    fclose(score_file);
}

int getMaxScore() {
    char in[9];
    int i = 0;
    FILE *score_file = fopen("scores", "rb");
    while (~(in[i] = (char) fgetc(score_file)))i++;
    in[i] = 0;
    fclose(score_file);
    return atoi(base64_decode(in, sizeof(in)));
}

#pragma clang diagnostic pop
