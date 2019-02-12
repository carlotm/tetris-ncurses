#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>

const int T = 1;
const int F = 0;
const char P[7][16] = {
    "..X...X...X...X.",
    "..X..XX..X......",
    ".X...XX...X.....",
    ".....XX..XX.....",
    "..X..XX...X.....",
    "..X...X..XX.....",
    ".XX...X...X.....",
};
const int C = 12;
const int R = 18;
struct state {
    short int over;
    int field[216];
    short int piece;
    short int rotation;
    int row;
    int col;
    short int fall;
    int speed;
    int speed_counter;
    int score;
    int lines;
    int level;
};


WINDOW* get_main_window();
void cleanup(WINDOW*);
void timing(struct state*);
void input(struct state*);
void logic(struct state*);
void render(struct state);
int rc2i(int, int);
int rotate(int, int, int);
int fit(struct state, int, int, int);
int fitCol(struct state, int);
int fitRow(struct state, int);
int fitRot(struct state, int);
char get_cell(struct state, int, int);
void create_new_piece(struct state*);
void check_for_rows(struct state*);
void fill(int, int, int);
void newgame(WINDOW*);
int menu(WINDOW*);


int main(int argc, char **argv)
{
    WINDOW *w = get_main_window();

    while(menu(w) == 0)
        newgame(w);

    cleanup(w);
    return 0;
}

WINDOW* get_main_window()
{
    WINDOW *w = initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_RED);
    init_pair(2, COLOR_GREEN, COLOR_GREEN);
    init_pair(3, COLOR_GREEN, COLOR_YELLOW);
    init_pair(4, COLOR_GREEN, COLOR_BLUE);
    init_pair(5, COLOR_GREEN, COLOR_MAGENTA);
    init_pair(6, COLOR_GREEN, COLOR_CYAN);
    init_pair(7, COLOR_GREEN, COLOR_WHITE);
    cbreak();
    noecho();
    keypad(w, true);
    nodelay(w, true);
    curs_set(0);
    refresh();
    return w;
}

void cleanup(WINDOW *w)
{
    endwin();
}

void timing(struct state *game)
{
    usleep(50 * 1000);
    game->speed_counter++;
    game->fall = (game->speed_counter == game->speed - (game->level * 2)) ? T : F;
}

void input(struct state *game)
{
    int k = getch();
    switch (k)
    {
        case KEY_LEFT:
            if (fitCol(*game, game->col - 1) == T)
                game->col--;
        break;
        case KEY_RIGHT:
            if (fitCol(*game, game->col + 1) == T)
                game->col++;
        break;
        case KEY_DOWN:
            if (fitRow(*game, game->row + 1) == T)
                game->row++;
        break;
        case KEY_UP:
            if (fitRot(*game, game->rotation + 1) == T)
                game->rotation++;
        break;
        default:
        break;
    }
}

void logic(struct state *game)
{
    if (game->fall == T)
    {
        if (fitRow(*game, game->row + 1))
            game->row++;
        else
        {
            /* Fix piece into field */
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    if (get_cell(*game, r, c) == 'X')
                    {
                        int temp_r = game->row + r;
                        int temp_c = game->col + c;
                        game->field[rc2i(temp_r, temp_c)] = game->piece + 1;
                    }

            check_for_rows(game);
            create_new_piece(game);
            game->over = fit(*game, game->rotation, game->row, game->col) == F ? T : F;
        }

        game->speed_counter = 0;
    }
}

void render(struct state game)
{
    int r = 0;
    int c = 0;

    /* Draw field */
    for (r = 0; r < R; r++)
        for (c = 0; c < C; c++)
            switch (game.field[rc2i(r, c)])
            {
                case 0:
                    mvprintw(r, c, " ");
                break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    fill(r, c, game.field[rc2i(r, c)]);
                break;
                case 9:
                    attron(A_REVERSE);
                    mvprintw(r, c, " ");
                    attroff(A_REVERSE);
                break;
                default:
                break;
            }

    /* Draw current piece */
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (get_cell(game, r, c) == 'X')
                fill(game.row + r, game.col + c, game.piece + 1);

    mvprintw(1, 14, "SCORE: %8d", game.score);
    mvprintw(2, 14, "LINES: %8d", game.lines);
    mvprintw(3, 14, "LEVEL: %8d", game.level);

    refresh();
}

char get_cell(struct state game, int r, int c)
{
    return P[game.piece][rotate(r, c, game.rotation)];
}

int rc2i(int r, int c)
{
    return r * C + c;
}

int rotate(int r, int c, int deg)
{
    switch (deg % 4)
    {
        case 0: return r * 4 + c;
        case 1: return 12 + r - (c * 4);
        case 2: return 15 - (r * 4) - c;
        case 3: return 3 - r + (c * 4);
    }
    return 0;
}

int fit(struct state game, int rot, int row, int col)
{
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
        {
            int pi = rotate(r, c, rot);
            int fi = rc2i(row + r, col + c);

            if (P[game.piece][pi] == 'X' && game.field[fi] != 0)
                return F;
        }

    return T;
}

int fitCol(struct state game, int col)
{
    return fit(game, game.rotation, game.row, col);
}

int fitRow(struct state game, int row)
{
    return fit(game, game.rotation, row, game.col);
}

int fitRot(struct state game, int rot)
{
    return fit(game, rot, game.row, game.col);
}

void create_new_piece(struct state *game)
{
    game->row = 0;
    game->col = C / 2;
    game->rotation = 0;
    game->piece = random() % 7;
    game->score += 25;
}

void check_for_rows(struct state* game)
{
    short int found_line;
    short int lines_count = 0;

    for (int r = 0; r < 4; r++)
        if (game->row + r < R - 1)
        {
            found_line = T;
            for (int c = 0; c < C; c++)
                if (game->field[rc2i(game->row + r, c)] == 0)
                    found_line = F;

            if (found_line == T)
            {
                lines_count++;
                game->lines++;

                for (int lr = (game->row + r) - 1; lr > 0; lr--)
                    for (int lc = 1; lc < C - 1; lc++)
                        game->field[rc2i(lr + 1, lc)] = game->field[rc2i(lr, lc)];

                game->level = game->lines / 10 + 1;
            }
        }

    game->score += lines_count * 25;
}

void fill(int r, int c, int color)
{
    attron(COLOR_PAIR(color));
    mvprintw(r, c, " ");
    attroff(COLOR_PAIR(color));
}

void newgame(WINDOW *w)
{
    struct state game = {
        F, // over
        {0}, // field
        0, // piece
        0, // rotation
        0, // row
        3, // col
        F, // fall
        22, // speed
        0, // speed_counter
        0, // score
        0, // lines
        1, // level
    };
    game.piece = random() % 7;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            game.field[rc2i(r, c)] = (c == 0 || c == C - 1 || r == R - 1) ? 9 : 0;

    wclear(w);
    nodelay(w, true);

    /* Game loop */
    while (game.over == F)
    {
        timing(&game);
        input(&game);
        logic(&game);
        render(game);
    }
}

int menu(WINDOW *w)
{
    char *menu[2] = {"New game", "Quit"};
    int current = 0;
    int choice;

    nodelay(w, false);
    wclear(w);

    while (T)
    {
        for (int i = 0; i < 2; i++)
            if (i == current)
                mvprintw(2 + i, 2, ">> %s <<", menu[i]);
            else
                mvprintw(2 + i, 2, "   %s   ", menu[i]);

        choice = getch();

        switch(choice)
        {
            case KEY_UP:
                current = current == 0 ? 1 : current - 1;
            break;
            case KEY_DOWN:
                current = current == 1 ? 0 : current + 1;
            break;
            case 10:
                return current;
            break;
            default:
            break;
        }
    }
}
