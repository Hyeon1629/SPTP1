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

typedef struct{
        int8_t x;
        int8_t y;
}position;

position snake[MAX_LENGTH];
int snake_length = 1;
position snake_dir;//snake_x,y_direoc
position* head = &snake[0];

position fruit;

position poison[MAX_POISON];

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
        int total_width = total_letters * letter_width + (total_letters - 1) * letter_spacing;
        int start_x = (WIDTH - total_width) / 2;
        int start_y = 3;

        clear();

        for (int row = 0; row <5; row++) {
                int x = start_x;
                for (int idx = 0; idx < total_letters; idx ++) {
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
                        for (int col = 0; col < letter_width; col ++) {
                                if (pattern[row][col] == '#') {
                                        mvaddch(start_y + row, x + col, ACS_CKBOARD);}
                                else {
                                        mvaddch(start_y + row, x + col, ' ');
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
        refresh();

        int ch;
        while ((ch = getch()) != '\n' && ch != '\r') { }
        nodelay(stdscr, TRUE);

        int mode = 0;
        int select_y = 13;
        const char *options[3] = {"EASY", "NORM", "HARD"};
        while (1) {
                const char *select_str = "SELECT GAME MODE";
                int select_x = (WIDTH - (int)strlen(select_str)) / 2;
                mvprintw(select_y, select_x, "%s", select_str);
                for (int i = 0; i < 3; i++) {
                        int option_y = select_y + 2 + i;
                        int option_x = (WIDTH - (int)strlen(options[i])) / 2;
                        if (i == mode) {
                                mvprintw(option_y, option_x - 2, ">%s", options[i]);
                        } else {
                                mvprintw(option_y, option_x - 2, " %s", options[i]);
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
                                case 0: pattern = font_G; break;
                                case 1: pattern = font_A; break;
                                case 2: pattern = font_M; break;
                                case 3: pattern = font_E; break;
                                case 4: pattern = font_SPACE; break;
                                case 5: pattern = font_O; break;
                                case 6: pattern = font_V; break;
                                case 7: pattern = font_E; break;
                                case 8: pattern = font_R; break;
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
        snprintf(buf, sizeof(buf), "SCORE: %d",eaten);
        int buf_x = (WIDTH - (int)strlen(buf)) / 2;
        mvprintw(start_y + 7, buf_x, "%s", buf);

        refresh();
        sleep(3);
}

/* clear bitmap array */
void clear_bitmap() {
        memset(bitmap, 0, WIDTH*HEIGHT);
}


void draw_bitmap() {
        move(0, 0);
        for (int i =0; i < WIDTH; i++) {
                move(0, i);
                addch(ACS_CKBOARD);
        }
        for (int i = 0; i < WIDTH; i++) {
                move(HEIGHT - 1, i);
                addch(ACS_CKBOARD);
        }

        for (int j = 0; j < HEIGHT; j++) {
                move(j, 0);
                addch(ACS_CKBOARD);
        }

        for (int j = 0; j < HEIGHT; j++) {
                move(j, WIDTH - 1);
                addch(ACS_CKBOARD);
        }


        for (int i = 1; i < HEIGHT - 1; i++) {
                for (int j = 1; j < WIDTH - 1; j ++) {
                        move(i, j);
                        int is_poison = 0;
                        for (int k = 0; k < poison_count; k ++) {
                                if (poison[k].x == j && poison[k].y == i) {
                                        is_poison = 1;
                                        break;
                                }
                        }
                        if (is_poison == 1) {
                                addch('X');
                        }
                        else if (fruit.x == j && fruit.y == i) {
                                addch('F');
                        }
                        else if (bitmap[j][i] == 1) {
                                addch(ACS_CKBOARD);
                        }
                        else {
                                addch(' ');
                        }
                }
        }
        refresh();
}

void process_input() {
        char ch = getch();

        int8_t new_dx = snake_dir.x;
        int8_t new_dy = snake_dir.y;

        if (ch == '\033') {
                getch();
                char arrow_key = getch();
                if (arrow_key == 'A') {
                        new_dy = -1;
                        new_dx = 0;
                }
                else if (arrow_key == 'B') {
                        new_dy = 1;
                        new_dx = 0;
                }
                else if (arrow_key == 'C') {
                        new_dx = 1;
                        new_dy = 0;
                }
                else if (arrow_key == 'D') {
                        new_dx = -1;
                        new_dy = 0;
                }
        }
        if (ch == 'w' || ch == 'W') {
                new_dy = -1;
                new_dx = 0;
        }
        if (ch == 's' || ch == 'S') {
                new_dy = 1;
                new_dx = 0;
        }
        if (ch == 'a' || ch == 'A') {
                new_dx = -1;
                new_dy = 0;
        }
        if (ch == 'd' || ch =='D') {
                new_dx = 1;
                new_dy = 0;
        }

        if (snake_length > 0) {
                int8_t next_x = head->x + new_dx;
                int8_t next_y = head->y + new_dy;
                if (next_x == snake[1].x && next_y == snake[1].y) {
                        return;
                }
        }
        snake_dir.x = new_dx;
        snake_dir.y = new_dy;
}

static bool occupied[HEIGHT][WIDTH];

void init_occupied() {
        memset(occupied, 0, sizeof(occupied));
        for (int i = 0; i < snake_length; i ++) {
                occupied[snake[i].y][snake[i].x] = true;
        }
}

void move_snake() {
        for (int i = snake_length - 1; i > 0; i --) {
                snake[i] = snake[i - 1];
                //tail_y[i] = tail_y[i - 1];
        }
        head->x += snake_dir.x;
        head->y += snake_dir.y;
        init_occupied();
}

/* draw the snake in the bitmap */
void mark_snake() {
        for (int i = 0; i < snake_length; i ++) {
                bitmap[snake[i].x][snake[i].y] = 1;
        }
}

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

void spawn_poison() {
        if (!free_cell()) return;
        for (int i = 0; i < poison_count; i++) {
                int8_t px, py;
                do {
                        px = (rand() % (WIDTH - 2)) + 1;
                        py = (rand() % (HEIGHT - 2)) + 1;
                } while (occupied[py][px]);
                poison[i].x = px;
                poison[i].y = py;
                occupied[py][px] = true;
        }
}

void init_entities() {
        while (1) {
                int8_t fx = (rand() % (WIDTH - 2)) + 1;
                int8_t fy = (rand() % (HEIGHT - 2)) + 1;
                if (fruit.x == head->x && fruit.y == head->y) {
                        continue;
                }
                int overlaps_tail = 0;
                for (int t = 0; t < snake_length; t ++) {
                        if (fruit.x == snake[t].x && fruit.y == snake[t].y) {
                                overlaps_tail = 1;
                                break;
                        }
                }
                if (overlaps_tail) continue;
                fruit.x = fx;
                fruit.y = fy;
                break;
        }
        if (poison_count > 0) {
                spawn_poison();
        }
}

void update_fruit_and_poison() {
        fruit_eaten++;
        if (snake_length < MAX_LENGTH - 1) {
                snake[snake_length].x = head->x;
                snake[snake_length].y = head->y;
                snake_length++;
                occupied[head->y][head->x] = true;
        }
        if (poison_count > 0 && fruit_eaten % poison_increase_step == 0 && poison_count < MAX_POISON) {
                poison_count++;
        }
        int8_t fx, fy;
        do {
                fx = (rand() % (WIDTH - 2)) + 1;
                fy = (rand() % (HEIGHT - 2)) + 1;
        } while (occupied[fy][fx]);

        fruit.x = fx;
        fruit.y = fy;
        occupied[fy][fx] = true;
        if (poison_count > 0) {
                spawn_poison();
        }
}

int check_gameover(){
    // gameover due to wall crash
    if (head->x <= 0 || head->x >= WIDTH - 1 || head->y >= HEIGHT - 1 || head->y <= 0) {
        return 1;
        }
    // gameover due to self_collision
    for (int i = 1; i < snake_length; i ++) {
        if(head->x == snake[i].x && head->y == snake[i].y) {
                return 1;
                }
        }
    // gameover due to poison
    for (int i = 0; i < poison_count; i ++) {
        if(head->x == poison[i].x && head->y == poison[i].y) {
                return 1;
                }
        }
        return 0;
}

int main() {
        head->x = 40;
        head->y = 12;
        snake_dir.x = 0;
        snake_dir.y = 1;
        initscr();              // initialize ncurse screen
        cbreak();               // disable the line break buffer
        nodelay(stdscr, TRUE);  // disable delay
        noecho();               // disable input character echos
        curs_set(0);            // disable cursor visibility
        //printf("\e[1j");//
        //keypad(stdscr, TRUE);

        nodelay(stdscr, FALSE);
        draw_title_screen();

        srand((unsigned)time(NULL));

        init_entities();
        init_occupied();

        clear_bitmap();
        mark_snake();
        draw_bitmap();
        sleep(3);

        while(1) {
                clear_bitmap();
                process_input();
                move_snake();

                if(check_gameover()){
                        draw_game_over_screen(fruit_eaten);
                        break;
                }
                if (head->x == fruit.x && head->y == fruit.y) {
                        update_fruit_and_poison();
                }
                mark_snake();
                draw_bitmap();
                usleep(100000); // sleep 100 ms
        }
        endwin();               // end the ncurs
        return 0;
}
