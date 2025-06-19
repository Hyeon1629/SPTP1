#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

#define WIDTH 80
#define HEIGHT 24
#define MAX_POISON 20
#define MAX_LENGTH 1000

/* Postion of the snake */
int8_t snake_x = 40;
int8_t snake_y = 12;

/* Moving directions of the snake */
int8_t snake_x_dir = 0;
int8_t snake_y_dir = 1;

int8_t tail_x[MAX_LENGTH];
int8_t tail_y[MAX_LENGTH];
int snake_length = 0;

/* directions of the fruit */
int8_t fruit_x;
int8_t fruit_y;

/* Position of poison */
int8_t poison_x[MAX_POISON];
int8_t poison_y[MAX_POISON];

int poison_count = 1;
int fruit_eaten = 0;
int poison_increase_step = 2;
int game_mode = 0;

/* bitmap to draw at the screen */
uint8_t bitmap[WIDTH][HEIGHT] = {0,};

/* dot-font */
static const char font_S[5][6] = {
        "#####",
        "#    ",
        "#####",
        "    #",
        "#####"
};

static const char font_N[5][6] = {
        "#   #",
        "##  #",
        "# # #",
        "#  ##",
        "#   #"
};

static const char font_A[5][6] = {
        " ### ",
        "#   #",
        "#####",
        "#   #",
        "#   #"
};

static const char font_K[5][6] = {
        "#   #",
        "#  # ",
        "###  ",
        "#  # ",
        "#   #"
};

static const char font_E[5][6] = {
        "#####",
        "#    ",
        "#####",
        "#    ",
        "#####"
};

static const char font_SPACE[5][6] = {
        "     ",
        "     ",
        "     ",
        "     ",
        "     "
};

static const char font_G[5][6] = {
        " ### ",
        "#    ",
        "#  ##",
        "#   #",
        " ####"
};

static const char font_M[5][6] = {
        "#   #",
        "## ##",
        "# # #",
        "#   #",
        "#   #"
};

static const char font_O[5][6] = {
        " ### ",
        "#   #",
        "#   #",
        "#   #",
        " ### "
};

static const char font_V[5][6] = {
        "#   #",
        "#   #",
        "#   #",
        " # # ",
        "  #  "
};

static const char font_R[5][6] = {
        "#### ",
        "#   #",
        "#### ",
        "#  # ",
        "#   #"
};

/* "SNAKE GAME" INTRO */
void draw_title_screen() {
        int total_letters = 10;
        int letter_width = 5;
        int letter_spacing = 1;
        int total_width = total_letters * letter_width + (total_letters -1) * letter_spacing;
        int start_x = (WIDTH - total_width) / 2;
        int start_y = 3;

        clear();

        for (int row = 0; row <5; row++) {
                int x = start_x;
                for (int idx = 0; idx < total_letters; idx++) {
                        const char (*pattern)[6];
                        switch (idx) {
                                case 0: pattern = font_S; break;
                                case 1: pattern = font_N; break;
                                case 2: pattern = font_A; break;
                                case 3: pattern = font_K; break;
                                case 4: pattern = font_E; break;
                                case 5: pattern = font_SPACE; break;
                                case 6: pattern = font_G; break;
                                case 7: pattern = font_A; break;
                                case 8: pattern = font_M; break;
                                case 9: pattern = font_E; break;
                                default: pattern = font_SPACE; break;
                        }

                        for (int col = 0; col < letter_width; col++) {
                                if (pattern[row][col] == '#') {
                                        mvaddch(start_y + row, x + col, ACS_CKBOARD);
                                } else {
                                        mvaddch(start_y + row, x+ col, ' ');
                                }
                        }
                        x += letter_width + letter_spacing;
                }
        }
        const char *press = "PRESS ENTER";
        int press_x = (WIDTH - (int)strlen(press)) / 2;
        mvprintw(start_y + 10, press_x, "%s", press);

        const char *footer = "@ 2025 MINHEE SEOKJIN JONGHYUN TAEJUN SANGGUK";
        int footer_x = (WIDTH - (int)strlen(footer)) / 2;
        mvprintw(HEIGHT - 2, footer_x, "%s", footer);

        refresh;
}

void draw_game_over_screen(int eaten) {
        int total_letters = 9;
        int letter_width = 5;
        int letter_spacing = 1;
        int total_width = total_letters * letter_width + (total_letters - 1) * letter_spacing;
        int start_x = (WIDTH - total_width) / 2;
        int start_y = 4;

        clear();
        for (int row = 0; row < 5; row ++) {
                int x = start_x;
                for (int idx = 0; idx < total_letters; idx ++) {
                        const char (*pattern)[6];
                        switch (idx) {
                                case 0: pattern = font_G;      break;
                                case 1: pattern = font_A;      break;
                                case 2: pattern = font_M;      break;
                                case 3: pattern = font_E;      break;
                                case 4: pattern = font_SPACE;  break;
                                case 5: pattern = font_O;      break;
                                case 6: pattern = font_V;      break;
                                case 7: pattern = font_E;      break;
                                case 8: pattern = font_R;      break;
                                default: pattern = font_SPACE; break;
                        }
                        for (int col = 0; col < letter_width; col ++) {
                                if (pattern[row][col] == '#') {
                                        mvaddch(start_y + row, x + col, ACS_CKBOARD);
                                } else {
                                        mvaddch(start_y + row, x + col, ' ');
                                }
                        }
                        x += letter_width + letter_spacing;
                }
        }
        char buf[32];
        snprintf(buf, sizeof(buf), "SCORE: %d", eaten);
        int buf_x = (WIDTH - (int)strlen(buf)) / 2;
        mvprintw(start_y + 7, buf_x, "%s", buf);

        refresh();
        sleep(3);
}

/* clear bitmap array */
void clear_bitmap() {
        memset(bitmap, 0, WIDTH*HEIGHT);
}

// 맵 그리기
void draw_bitmap() {
        move(0, 0);             // 커서 맨위로 이동

        // 위쪽 테두리
        for (int i =0; i < WIDTH; i++) // i = 행
        {
                move(0, i);
                addch(ACS_CKBOARD); // 체커보드 문자
        }

        // 아래쪽 테두리
        for (int i = 0; i < WIDTH; i++) 
        {
                move(HEIGHT - 1, i);
                addch(ACS_CKBOARD);
        }

        // 왼쪽 테두리
        for (int j = 0; j < HEIGHT; j++) // j = 열
        {
                move(j, 0);
                addch(ACS_CKBOARD);
        }

        // 오른쪽 테두리
        for (int j = 0; j < HEIGHT; j++)
        {
                move(j, WIDTH - 1);
                addch(ACS_CKBOARD);
        }

        // 안쪽 시작

        for (int i = 1; i < HEIGHT - 1; i++)
        {
                for (int j = 1; j < WIDTH - 1; j++)
                {
                        move(i, j)

                        int is_poison = 0;

                        for (int k = 0; k < poison_count; k++)
                        {
                                if (poison[k].x == j && poison[k].y == i)
                                {
                                        is_poison = 1;
                                        break;
                                }
                        }
                        
                        if (is_poison == 1)
                        {
                                addch('X');
                        }

                        else if (fruit.x == j && fruit.y == i)
                        {
                                addch('F');
                        }

                        else if (bitmap[j][i] == 1)
                        {
                                addch(ACS_CKBOARD);
                        }

                        else
                        {
                                addch(' ');
                        }
                        
                }
        }

        refresh();
}

// 키보드 입력
void process_input()
{
        char ch = getch(); // 키보드 입력 받기

        // 전진을 위한 방향값 복사 변수
        int8_t new_dx = head->x_dir; // 현재 x방향 복사
        int8_t new_dy = head->y_dir; // 현재 y방향 복사

        // 방향키 파트, 화살표 입력 시퀀스는 '\033[' 뒤에 위,아래,오른쪽,왼쪽 순으로 A,B,C,D 가 붙는다.

        if (ch == '\033') { // 방향키인지 확인

                getch(); // '[' 문자를 스킵
                char arrow_key = getch(); // 방향키 문자 입력 받기

                if (arrow_key == 'A')
                {
                        new_dy = -1;
                }

                else if (arrow_key == 'B') // 아래 방향키
                {
                        new_dy = 1;
                }

                else if (arrow_key == 'C') // 오른쪽 방향키
                {
                        new_dx = 1;
                }

                else if (arrow_key == 'D') // 왼쪽 방향키
                {
                        new_dx = -1;
                }
        }

        if (ch == 'w' || ch == 'W') // W키 입력시
        {
                new_dy = -1;
        }

        if (ch == 's' || ch == 'S') // S키 입력시
        {
                new_dy = 1;
        }

        if (ch == 'a' || ch == 'A') // A키 입력시
        {
                new_dx = -1;
        }

        if (ch == 'd' || ch =='D') // D키 입력시
        {
                new_dx = 1;
        }

        // 
        if (snake_length > 0) // 꼬리가 있으면
        {
                int8_t next_x = head->x + new_dx; // 머리의 다음 위치 계산
                int8_t next_y = head->y + new_dy; // 

                if (next_x == snake[1].x && next_y == snake[1].y) // 머리 다음위치가 꼬리 위치라면
                {
                        return; // 움직임 종료
                }
        }


        head->x_dir = new_dx; // 다음 방향 적용
        head->y_dir = new_dy; // 
}


int check_self_collision() {
        for (int i = 0; i < snake_length; i ++) {
                if (snake_x == tail_x[i] && snake_y == tail_y[i]) {
                        return 1;
                }
        }
        return 0;
}

/* for every cycle, move the snake postion diagonally */
void move_snake() {
        for (int i = snake_length - 1; i > 0; i --) {
                snake[i] = snake[i - 1];
                //tail_y[i] = tail_y[i - 1];
        }
        head->x += snake_dir.x;
        head->y += snake_dir.y;

        if (head->x <= 0 || head->x >= WIDTH - 1 || head->y <= 0 || head->y >= HEIGHT - 1) {
                draw_game_over_screen(fruit_eaten);
                endwin();
                exit(0);
        }
}


/* draw the snake in the bitmap */
void mark_snake() {
        for (int i = 0; i < snake_length; i ++) {
                bitmap[tail_x[i]][tail_y[i]] = 1;
        }
}

// 전역변수
static bool occupied[HEIGHT][WIDTH];

// 맵 내부에 빈 칸이 하나라도 있는지 확인
static bool free_cell() {
    for (int y = 1; y < HEIGHT - 1; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            if (!occupied[y][x]) {
                return true;
            }
        }
    }
    return false;
}

// 게임 시작 시 한 번만 호출해서 occupied 초기화
void init_occupied() {
    // 모든 칸 false(비어 있음)로 초기화
    memset(occupied, 0, sizeof(occupied));
    occupied[snake_y][snake_x] = true;
    // 뱀 머리 초기 위치 찍기
}

// poison_count 개수만큼 독을 맵의 빈 칸에 무작위 배치
void spawn_poison() {
    // 빈 칸 없으면 바로 리턴
    if (!free_cell()) return; 

    for (int i = 0; i < poison_count; i++) {
        int8_t px, py;
        // 빈 칸이 나올 때까지 반복
        do {
            px = (rand() % (WIDTH - 2)) + 1;  
            py = (rand() % (HEIGHT - 2)) + 1; 
            // occupied[py][px]가 true면 이미 뱀, 과일, 또는 이전 독이 차지 중인것
        } while (occupied[py][px]);
        // 안전한 좌표[px,py]를 찾았으면 저장
        poison_x[i] = px;
        poison_y[i] = py;
        // 해당 칸을 점유 처리해서 중복 배치를 방지
        occupied[py][px] = true;
    }
}

// 과일을 먹었을 때 : 뱀 길이 증가, 과일 재배치, 독 재배치
void update_fruit_and_poison() {
    fruit_eaten++;

    // 뱀 길이 늘리기
    if (snake_length < MAX_LENGTH - 1) {
        // 꼬리 배열 끝에 새 머리 좌표 추가
        tail_x[snake_length] = snake_x;
        tail_y[snake_length] = snake_y;
        snake_length++;
        // 뱀 새 위치 점유
        occupied[snake_y][snake_x] = true;
    }

    // 난이도에 따라 독 개수 증가
    if (poison_count > 0
        && fruit_eaten % poison_increase_step == 0
        && poison_count < MAX_POISON) {
        poison_count++;
    }

    // 새 과일 위치 찾기 (빈 칸일 때까지)
    int8_t fx, fy;
    do {
        fx = (rand() % (WIDTH - 2)) + 1;
        fy = (rand() % (HEIGHT - 2)) + 1;
    } while (occupied[fy][fx]);
    
    fruit_x = fx;
    fruit_y = fy;
    occupied[fy][fx] = true; //과일 위치 찍기

    // 독도 새로 배치
    if (poison_count > 0) {
        spawn_poison();
    }
}

int check_poison_collision() {
    for (int i = 0; i < poison_count; i++) {
        if (snake_x == poison_x[i] && snake_y == poison_y[i]) {
            return 1;
        }
    }
    return 0;
}

int main() {
        initscr();              // initialize ncurse screen
        cbreak();               // disable the line break buffer
        nodelay(stdscr, TRUE);  // disable delay to wait keyboard inputs
        noecho();               // disable input character echos
        curs_set(0);            // disable cursor visibility
        //printf("\e[1j");//
        //keypad(stdscr, TRUE);

        nodelay(stdscr, FALSE);
        draw_title_screen();

        int ch;
        while ((ch = getch()) != '\n' && ch != '\r') {
        }
        nodelay(stdscr, TRUE);

        int mode = 0;
        int start_y = 3;
        int select_y = start_y +10;
        const char *options[3] = {"EASY", "NORM", "HARD"};

        while (1) {
                const char *select_str = "SELECT GAME MODE";
                int select_x = (WIDTH - (int)strlen(select_str)) / 2;
                mvprintw(select_y, select_x, "%s", select_str);

                for (int i = 0; i < 3; i++) {
                        int option_y = select_y + 2 + i;
                        int option_x = (WIDTH - (int)strlen(options[i])) / 2;
                        if (i == mode) {
                                mvprintw(option_y, option_x - 2, "> %s", options[i]);
                        } else {
                                mvprintw(option_y, option_x - 2, "  %s", options[i]);
                        }
                }
                refresh();

                int c = getch();
                if (c == '\t') {
                        mode = (mode + 1) % 3;
                }
                else if (c == '\n' || c == '\r') {
                        break;
                }
                usleep(50000);
        }

        game_mode = mode;
        switch (mode) {
                case 0:
                        poison_increase_step = 256;
                        poison_count = 0;
                        break;
                case 1:
                        poison_increase_step = 5;
                        poison_count = 1;
                        break;
                case 2:
                        poison_increase_step = 1;
                        poison_count = 1;
                        break;
        }
        srand((unsigned)time(NULL));

        while (1) {
                int8_t fx = (rand() % (WIDTH - 2)) + 1;
                int8_t fy = (rand() % (HEIGHT - 2)) + 1;
                if (fruit_x == snake_x && fruit_y == snake_y) {
                        continue;
                }
                int overlaps_tail = 0;
                for (int t = 0; t < snake_length; t ++) {
                        if (fruit_x == tail_x[t] && fruit_y == tail_y[t]) {
                                overlaps_tail = 1;
                                break;
                        }
                }
                if (overlaps_tail) {
                        continue;
                }
                fruit_x = fx;
                fruit_y = fy;
                break;
        }
        if (poison_count > 0) {
                spawn_poison();
        }
        clear_bitmap();
        mark_snake();
        draw_bitmap();
        sleep(3);

        while(1) {
                clear_bitmap();
                process_input();
                move_snake();

                if (check_self_collision()) {
                        draw_game_over_screen(fruit_eaten);
                        endwin();
                        exit(0);
                }

                if (snake_x == fruit_x && snake_y == fruit_y) {
                        update_fruit_and_poison();
                }

                if (poison_count > 0 && check_poison_collision()) {
                        draw_game_over_screen(fruit_eaten);
                        endwin();
                        exit(0);
                }
                mark_snake();
                draw_bitmap();
                usleep(100000); // sleep 100 ms
        }
        endwin();               // end the ncurses screen
}
