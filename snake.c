#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
//123123123123
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

/* draw bitmap array on the screen */
void draw_bitmap() {
        move(0, 0);             //move cursor to the top left corner
        for (int j = 0; j < WIDTH; j ++) {
                mvaddch(0, j, ACS_CKBOARD);
                mvaddch(HEIGHT - 1, j, ACS_CKBOARD);
        }

        for (int i = 1; i < HEIGHT - 1; i++) {
                mvaddch(i, 0, ACS_CKBOARD);
                for (int j = 1; j < WIDTH - 1; j++) {
                        int is_poison = 0;
                        for (int k = 0; k < poison_count; k++) {
                                if (j == poison_x[k] && i == poison_y[k]) {
                                        addch('X');
                                        is_poison = 1;
                                        break;
                                }
                        }
                        if (is_poison) continue;
                        /* fruit is F */
                        if (j == fruit_x && i == fruit_y) {
                                addch('F');
                        }
                        else if (bitmap[j][i]) {
                                addch(ACS_CKBOARD);     //black
                        } else {
                                addch(' ');             //white
                        }
                }
                mvaddch(i, WIDTH - 1, ACS_CKBOARD);
        }
        refresh();
}

/* process keyboard inputs */
void process_input() {
        char ch = getch();
        int8_t new_dx = snake_x_dir;
        int8_t new_dy = snake_y_dir;

        /* special character starting with 'ESC' */
        if (ch == '\033') {
                getch(); // skip '['
                switch(getch()) {
                        case 'A':       // arrow up
                                new_dx = 0;
                                new_dy = -1;
                                break;
                        case 'B':       // arrow down
                                new_dx = 0;
                                new_dy = 1;
                                break;
                        case 'C':       // arrow right
                                new_dx = 1;
                                new_dy = 0;
                                break;
                        case 'D':       // arrow left
                                new_dx = -1;
                                new_dy = 0;
                                break;
                        }
        } else if (ch == 'w' || ch == 'W') {
                new_dx = 0; new_dy = -1;}       // 'w' key
        else if (ch == 's' || ch == 'S') {
                new_dx = 0; new_dy = 1;}        // 's' key
        else if (ch == 'a' || ch == 'A') {
                new_dx = -1; new_dy = 0;}       // 'a' key
        else if (ch == 'd' || ch == 'D') {
                new_dx = 1; new_dy = 0;}        // 'd' key

        if (snake_length > 0) {
                int8_t next_x = snake_x + new_dx;
                int8_t next_y = snake_y + new_dy;
                if (next_x == tail_x[0] && next_y == tail_y[0]) {
                        return;
                }
        }
        snake_x_dir = new_dx;
        snake_y_dir = new_dy;

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
                tail_x[i] = tail_x[i - 1];
                tail_y[i] = tail_y[i - 1];
        }

        if (snake_length > 0) {
                tail_x[0] = snake_x;
                tail_y[0] = snake_y;
        }

        snake_x += snake_x_dir;
        snake_y += snake_y_dir;

        if (snake_x <= 0 || snake_x >= WIDTH - 1 || snake_y <= 0 || snake_y >= HEIGHT - 1) {
                draw_game_over_screen(fruit_eaten);
                endwin();
                exit(0);
        }
}

/* draw the snake in the bitmap */
void mark_snake() {
        bitmap[snake_x][snake_y] = 1;

        for (int i = 0; i < snake_length; i ++) {
                bitmap[tail_x[i]][tail_y[i]] = 1;
        }
}

void spawn_poison() {
        for (int i = 0; i < poison_count; i++) {
                int8_t px, py;
                while (1) {
                        px = (rand() % (WIDTH - 2)) + 1;
                        py = (rand() % (HEIGHT - 2)) + 1;

                        if (px == fruit_x && py == snake_y) {
                                continue;
                        }

                        int overlaps_tail = 0;
                        for (int t = 0; t < snake_length; t ++) {
                                if (px == tail_x[t] && py == tail_y[t]) {
                                        overlaps_tail = 1;
                                        break;
                                }
                        }
                        if (overlaps_tail) {
                                continue;
                        }
                        break;
                }
                poison_x[i] = px;
                poison_y[i] = py;
        }
}

void update_fruit_and_poison() {
    fruit_eaten ++;

    if (snake_length < MAX_LENGTH - 1) {
            tail_x[snake_length] = snake_x;
            tail_y[snake_length] = snake_y;
            snake_length ++;
    }

    if (poison_count > 0 && fruit_eaten % poison_increase_step == 0 && poison_count < MAX_POISON) {
        poison_count ++;
        }
    while (1) {
            int8_t fx = (rand() % (WIDTH - 2)) + 1;
            int8_t fy = (rand() % (HEIGHT - 2)) + 1;
            if (fx == snake_x && fy == snake_y) {
                    continue;
            }
            int overlaps_tail = 0;
            for (int t = 0; t < snake_length; t ++) {
                    if (fx == tail_x[t] && fy == tail_y[t]) {
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
