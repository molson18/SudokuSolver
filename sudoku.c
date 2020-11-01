/**
 * sudoku.c
 *
 * CS50 AP
 * Sudoku
 *
 * Implements the game of Sudoku.
 */

#include <ctype.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku.h"
#include <time.h>

// macro for processing control characters
#define CTRL(x) ((x) & ~0140)

// size of each int (in bytes) in *.bin files
#define INTSIZE 4

// wrapper for our game's globals
struct
{
    // the current level
    char *level;

    // the game's board
    int board[9][9];

    // given game board
    int og[9][9];

    // the board's number
    int number;

    // the board's top-left coordinates
    int top, left;

    // the cursor's current location between (0,0) and (8,8)
    int y, x;

    // if the game has been won
    bool win;

    // for activating hints
    bool hint;

    // for possible moves
    int poss[9][9][9];
} g;

// infinite undo's
typedef struct node
{
    // storing y, x, and value of previous moves
    int storage[3];

    struct node* next;
}
node;

node* head = NULL;

// prototypes
void draw_grid(void);
void draw_borders(void);
void draw_logo(void);
void draw_numbers(void);
void hide_banner(void);
bool load_board(void);
void handle_signal(int signum);
void log_move(int ch);
void redraw_all(void);
bool restart_game(void);
void show_banner(char *b);
void show_cursor(void);
void shutdown(void);
bool startup(void);

// newly-added prototypes
void check(void);
void checkrows(int val);
void checkcols(int val);
void checkboxes(int val);
bool check_ind_box(int x, int y, int val);
void checkwin(void);
void reposition(int ch);
void update_numbers(int ch, bool zero);
void undo_move(void);

// added features
bool checkMove(int move);
bool checkWin(void);
void append(void);
void deleteNode(void);
void possMoves(void);

int main(int argc, char *argv[])
{
    // ensure that number of arguments is as expected
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage: sudoku n00b|l33t [#]\n");
        return 1;
    }

    // ensure that level is valid
    if (strcmp(argv[1], "debug") == 0)
    {
        g.level = "debug";
    }
    else if (strcmp(argv[1], "n00b") == 0)
    {
        g.level = "n00b";
    }
    else if (strcmp(argv[1], "l33t") == 0)
    {
        g.level = "l33t";
    }
    else
    {
        fprintf(stderr, "Usage: sudoku n00b|l33t [#]\n");
        return 2;
    }


    // n00b and l33t levels have 1024 boards; debug level has 9
    int max = (strcmp(g.level, "debug") == 0) ? 9 : 1024;

    // ensure that #, if provided, is in [1, max]
    if (argc == 3)
    {
        // ensure n is integral
        char c;
        if (sscanf(argv[2], " %d %c", &g.number, &c) != 1)
        {
            fprintf(stderr, "Usage: sudoku n00b|l33t [#]\n");
            return 3;
        }

        // ensure n is in [1, max]
        if (g.number < 1 || g.number > max)
        {
            fprintf(stderr, "That board # does not exist!\n");
            return 4;
        }

        // seed PRNG with # so that we get same sequence of boards
        srand(g.number);
    }
    else
    {
        // seed PRNG with current time so that we get any sequence of boards
        srand(time(NULL));

        // choose a random n in [1, max]
        g.number = rand() % max + 1;
    }

    // start up ncurses
    if (!startup())
    {
        fprintf(stderr, "Error starting up ncurses!\n");
        return 5;
    }

    // register handler for SIGWINCH (SIGnal WINdow CHanged)
    signal(SIGWINCH, (void (*)(int)) handle_signal);

    // start the first game
    if (!restart_game())
    {
        shutdown();
        fprintf(stderr, "Could not load board from disk!\n");
        return 6;
    }
    redraw_all();

    // let the user play!
    int ch;
    do
    {
        // refresh the screen
        refresh();

        // get user's input
        ch = getch();

        // capitalize input to simplify cases
        ch = toupper(ch);

        // process user's input
        switch (ch)
        {
            // start a new game
            case 'N':
            {
                g.number = rand() % max + 1;
                if (!restart_game())
                {
                    shutdown();
                    fprintf(stderr, "Could not load board from disk!\n");
                    return 6;
                }
                g.win = false;
                redraw_all();
                break;
            }

            // restart current game
            case 'R':
            {
                if (!restart_game())
                {
                    shutdown();
                    fprintf(stderr, "Could not load board from disk!\n");
                    return 6;
                }
                g.win = false;
                redraw_all();
                break;
            }

            // let user manually redraw screen with ctrl-L
            case CTRL('l'):
                redraw_all();
                break;

            case KEY_UP:
            {
                g.y = (g.y + 8) % 9; // equivilent to -1 unless 0
                show_cursor();
                break;
            }

            case KEY_DOWN:
            {
                g.y = (g.y + 1) % 9; // % 9 so that it wraps around
                show_cursor();
                break;
            }

            case KEY_LEFT:
            {
                g.x = (g.x + 8) % 9; // for wrap around
                show_cursor();
                break;
            }

            case KEY_RIGHT:
            {
                g.x = (g.x + 1) % 9; // for wrap around
                show_cursor();
                break;
            }
        }

        // undoing last move
        if (ch == 'U' || ch == CTRL('z'))
        {
            if (head != NULL)
            {
                node* end = malloc(sizeof(node));
                end = head;
                while (end->next != NULL)
                {
                    end = end->next;
                }
                g.board[end->storage[0]][end->storage[1]] = end->storage[2];
                deleteNode();
                draw_numbers();
                show_cursor();
            }
            else
            {
                show_banner("CAN'T UNDO");
                show_cursor();
            }
        }

        // so that '.', backspace, and delete are equivlent to 0
        if (ch == '.' || ch == KEY_BACKSPACE || ch == KEY_DC)
        {
            ch = '0';
        }

        // sets the space equal to input as long as it was not given at start
        if (ch >= '0' && ch <= '9' && g.og[g.y][g.x] == 0 && !g.win)
        {
            if (checkMove((int)ch - '0'))
            {
                append();
                g.board[g.y][g.x] = ch - '0';
                draw_numbers();
                hide_banner(); // remove the error message
                show_cursor();
            }
            else
            {
                show_banner("NOT A VALID MOVE, TRY AGAIN"); // show error message
                show_cursor();
            }
        }

        // log input (and board's state) if any was received this iteration
        if (ch != ERR)
        {
            log_move(ch);
        }

        // only enters the first time, to prevent flickering
        if (g.win == false && checkWin())
        {
            g.win = checkWin();
            redraw_all();
            show_banner("CONGRATULATIONS, YOU WON!");
            show_cursor();
        }

    }
    while (ch != 'Q');

    // shut down ncurses
    shutdown();

    // tidy up the screen (using ANSI escape sequences)
    printf("\033[2J");
    printf("\033[%d;%dH", 0, 0);

    // that's all folks
    printf("\nkthxbai!\n\n");
    return 0;
}


/*
 * Draw's the game's board.
 */
void draw_grid(void)
{
    // get window's dimensions
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // determine where top-left corner of board belongs
    g.top = maxy / 2 - 7;
    g.left = maxx / 2 - 30;

    // enable color if possible
    if (has_colors())
    {
        attron(COLOR_PAIR(PAIR_GRID));
    }

    // changes grid to green on win
    if (has_colors() && g.win)
    {
        attron(COLOR_PAIR(PAIR_WONGRID));
    }

    // print grid
    for (int i = 0 ; i < 3 ; ++i )
    {
        mvaddstr(g.top + 0 + 4 * i, g.left, "+-------+-------+-------+");
        mvaddstr(g.top + 1 + 4 * i, g.left, "|       |       |       |");
        mvaddstr(g.top + 2 + 4 * i, g.left, "|       |       |       |");
        mvaddstr(g.top + 3 + 4 * i, g.left, "|       |       |       |");
    }
    mvaddstr(g.top + 4 * 3, g.left, "+-------+-------+-------+" );

    // remind user of level and #
    char reminder[maxx + 1];
    sprintf(reminder, "   playing %s #%d", g.level, g.number);
    mvaddstr(g.top + 14, g.left + 25 - strlen(reminder), reminder);

    // disable color if possible
    if (has_colors())
    {
        attroff(COLOR_PAIR(PAIR_GRID));
    }
}


/*
 * Draws game's borders.
 */
void draw_borders(void)
{
    // get window's dimensions
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // enable color if possible (else b&w highlighting)
    if (has_colors())
    {
        attron(A_PROTECT);
        attron(COLOR_PAIR(PAIR_BORDER));
    }
    else
    {
        attron(A_REVERSE);
    }

    // changes grid to green on win
    if (has_colors() && g.win)
    {
        attron(COLOR_PAIR(PAIR_WONBORDER));
    }

    // draw borders
    for (int i = 0; i < maxx; i++)
    {
        mvaddch(0, i, ' ');
        mvaddch(maxy - 1, i, ' ');
    }



    // draw header
    char header[maxx + 1];
    sprintf(header, "%s by %s", TITLE, AUTHOR);
    mvaddstr(0, (maxx - strlen(header)) / 2, header);

    // draw footer
    mvaddstr(maxy - 1, 1, "[N]ew Game   [R]estart Game");
    mvaddstr(maxy - 1, maxx - 13, "[Q]uit Game");

    // disable color if possible (else b&w highlighting)
    if (has_colors())
    {
        attroff(COLOR_PAIR(PAIR_BORDER));
    }
    else
    {
        attroff(A_REVERSE);
    }
}


/*
 * Draws game's logo.  Must be called after draw_grid has been
 * called at least once.
 */
void draw_logo(void) // bool for invalid move
{
    // determine top-left coordinates of logo
    int top = g.top + 2;
    int left = g.left + 30;

    // enable color if possible
    if (has_colors())
    {
        attron(COLOR_PAIR(PAIR_LOGO));
    }

    // draw logo
    mvaddstr(top + 0, left, "               _       _          ");
    mvaddstr(top + 1, left, "              | |     | |         ");
    mvaddstr(top + 2, left, " ___ _   _  __| | ___ | | ___   _ ");
    mvaddstr(top + 3, left, "/ __| | | |/ _` |/ _ \\| |/ / | | |");
    mvaddstr(top + 4, left, "\\__ \\ |_| | (_| | (_) |   <| |_| |");
    mvaddstr(top + 5, left, "|___/\\__,_|\\__,_|\\___/|_|\\_\\\\__,_|");

    // sign logo
    char signature[3 + strlen(AUTHOR) + 1];
    sprintf(signature, "by %s", AUTHOR);
    mvaddstr(top + 7, left + 35 - strlen(signature) - 1, signature);

    // congratulations on win
    // if (g.win)
    // {
    //     mvaddstr(top - 1, left, "CONGRATULATIONS, YOU WON!");
    // }

    // disable color if possible
    if (has_colors())
    {
        attroff(COLOR_PAIR(PAIR_LOGO));
    }
}


/*
 * Draw's game's numbers.  Must be called after draw_grid has been
 * called at least once.
 */
void draw_numbers(void)
{
    // enable color if possible
    // starts with win color
    if (has_colors())
    {
        attron(COLOR_PAIR(PAIR_WONDIGITS));
    }

    // user typed color
    if (has_colors() && g.win == false)
    {
        attron(COLOR_PAIR(PAIR_DIGITS));
    }

    // iterate over board's numbers
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            // determine char
            if (g.og[i][j] == 0) // only for user typed numbers
            {
                char c = (g.board[i][j] == 0) ? '.' : g.board[i][j] + '0';
                mvaddch(g.top + i + 1 + i / 3, g.left + 2 + 2 * (j + j / 3), c);
                refresh();
            }
        }
    }

    // original board in yellow
    if (has_colors() && g.win == false)
    {
        attron(COLOR_PAIR(PAIR_OGDIGITS));
    }


    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            // only prints the original characters
            if (g.og[i][j] != 0)
            {
                char c = (g.board[i][j] == 0) ? '.' : g.board[i][j] + '0';
                mvaddch(g.top + i + 1 + i / 3, g.left + 2 + 2 * (j + j / 3), c);
                refresh();
            }
        }
    }

    // disable color if possible
    if (has_colors())
    {
        attroff(COLOR_PAIR(PAIR_BANNER));
    }
}


/*
 * Designed to handles signals (e.g., SIGWINCH).
 */
void handle_signal(int signum)
{
    // handle a change in the window (i.e., a resizing)
    if (signum == SIGWINCH)
    {
        redraw_all();
    }

    // re-register myself so this signal gets handled in future too
    signal(signum, (void (*)(int)) handle_signal);
}


/*
 * Hides banner.
 */
void hide_banner(void)
{
    // get window's dimensions
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // overwrite banner with spaces
    for (int i = g.left + 30; i < maxx; i++)
    {
        mvaddch(g.top + 1, i, ' ');
    }
}


/*
 * Loads current board from disk, returning true iff successful.
 */
bool load_board(void)
{
    // open file with boards of specified level
    char filename[strlen(g.level) + 5];
    sprintf(filename, "%s.bin", g.level);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        return false;
    }

    // determine file's size
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);

    // ensure file is of expected size
    if (size % (81 * INTSIZE) != 0)
    {
        fclose(fp);
        return false;
    }

    // compute offset of specified board
    int offset = ((g.number - 1) * 81 * INTSIZE);

    // seek to specified board
    fseek(fp, offset, SEEK_SET);

    // read board into memory
    if (fread(g.board, 81 * INTSIZE, 1, fp) != 1)
    {
        fclose(fp);
        return false;
    }

    // w00t
    fclose(fp);
    return true;
}


/*
 * Logs input and board's state to log.txt to facilitate automated tests.
 */
void log_move(int ch)
{
    // open log
    FILE *fp = fopen("log.txt", "a");
    if (fp == NULL)
    {
        return;
    }

    // log input
    fprintf(fp, "%d\n", ch);

    // log board
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            fprintf(fp, "%d", g.board[i][j]);
        }
        fprintf(fp, "\n");
    }

    // that's it
    fclose(fp);
}


/*
 * (Re)draws everything on the screen.
 */
void redraw_all(void)
{
    // reset ncurses
    endwin();
    refresh();

    // clear screen
    clear();

    // re-draw everything
    draw_borders();
    draw_grid();
    draw_logo(); // valid move
    draw_numbers();

    // show cursor
    show_cursor();
}


/*
 * (Re)starts current game, returning true iff succesful.
 */
bool restart_game(void)
{
    // reload current game
    if (!load_board())
    {
        return false;
    }

    // storing original board
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            g.og[i][j] = g.board[i][j];
        }
    }

    // redraw board
    draw_grid();
    draw_numbers();

    // get window's dimensions
    int maxy, maxx;
    getmaxyx(stdscr, maxy, maxx);

    // move cursor to board's center
    g.y = g.x = 4;
    show_cursor();

    // remove log, if any
    remove("log.txt");

    if (head != NULL)
    {
        while (head != NULL)
        {
            deleteNode();
        }
    }
    // w00t
    return true;
}


/*
 * Shows cursor at (g.y, g.x).
 */
void show_cursor(void)
{
    // restore cursor's location
    move(g.top + g.y + 1 + g.y / 3, g.left + 2 + 2 * (g.x + g.x / 3));
}


/*
 * Shows a banner.  Must be called after show_grid has been
 * called at least once.
 */
void show_banner(char *b)
{
    // enable color if possible
    if (has_colors())
    {
        attron(COLOR_PAIR(PAIR_BANNER));
    }

    if (has_colors() && g.win)
    {
        attron(COLOR_PAIR(PAIR_WONDIGITS));
    }

    // determine where top-left corner of board belongs
    mvaddstr(g.top + 1, g.left + 60 - strlen(b), b);

    // disable color if possible
    if (has_colors())
    {
        attroff(COLOR_PAIR(PAIR_BANNER));
    }
}


/*
 * Shuts down ncurses.
 */
void shutdown(void)
{
    endwin();
}


/*
 * Starts up ncurses.  Returns true iff successful.
 */
bool startup(void)
{
    // initialize ncurses
    if (initscr() == NULL)
    {
        return false;
    }

    // prepare for color if possible
    if (has_colors())
    {
        // enable color
        if (start_color() == ERR || attron(A_PROTECT) == ERR)
        {
            endwin();
            return false;
        }
        // initialize pairs of colors
        if (init_pair(PAIR_BANNER, FG_BANNER, BG_BANNER) == ERR ||
            init_pair(PAIR_GRID, FG_GRID, BG_GRID) == ERR ||
            init_pair(PAIR_BORDER, FG_BORDER, BG_BORDER) == ERR ||
            init_pair(PAIR_LOGO, FG_LOGO, BG_LOGO) == ERR ||
            init_pair(PAIR_DIGITS, FG_DIGITS, BG_DIGITS) == ERR ||
            init_pair(PAIR_OGDIGITS, COLOR_YELLOW, BG_DIGITS) == ERR ||
            init_pair(PAIR_WONDIGITS, COLOR_GREEN, BG_DIGITS) == ERR ||
            init_pair(PAIR_WONBORDER, FG_BORDER, COLOR_GREEN) == ERR ||
            init_pair(PAIR_WONGRID, COLOR_GREEN, BG_GRID) == ERR)
        {
            endwin();
            return false;
        }
    }

    // don't echo keyboard input
    if (noecho() == ERR)
    {
        endwin();
        return false;
    }

    // disable line buffering and certain signals
    if (raw() == ERR)
    {
        endwin();
        return false;
    }

    // enable arrow keys
    if (keypad(stdscr, true) == ERR)
    {
        endwin();
        return false;
    }

    // wait 1000 ms at a time for input
    timeout(1000);

    // w00t
    return true;
}

bool checkWin(void)
{
    // sum for checking since as long as the sum is 45 and it passes checkMove it has been won
    int sum = 0;
    // check rows and columns, the sum of both should be 90
    for (int i = 0; i < 9; i++) // every row/column
    {
        for (int j = 0; j < 9; j++) // each spot in a row/column
        {
            sum += g.board[i][j];
            sum += g.board[j][i];
        }
        if (sum != 90)
        {
            return false;
        }
        sum = 0;
    }
    // check boxes
    for (int i = 0; i < 9; i++) // for every box
    {
        for (int j = 0; j < 9; j++) // every spot in every box
        {
            sum += g.board[3 * (i / 3) + j / 3][3 * (i % 3) + j % 3];
        }
        if (sum != 45)
        {
            return false;
        }
        sum = 0;
    }
    return true;
}

bool checkMove(int move)
{
    // if 0, is a good move
    if (move == 0)
    {
        return true;
    }
    // check rows and columns
    for (int i = 0; i < 9; i++)
    {
        if ((g.board[g.y][i] == move && i != g.x) ||
            (g.board[i][g.x] == move && i != g.y))
        {
            return false;
        }
    }
    // check boxes
    for (int i = 0; i < 9; i++)
    {
        if (g.board[3 * (g.y / 3) + i / 3][3 * (g.x / 3) + i % 3] == move
            && 3 * (g.y / 3) + i / 3 != g.y && 3 * (g.x / 3) + i % 3 != g.x)
        {
            return false;
        }
    }
    return true;
}

void append(void)
{
    node* temp = malloc(sizeof(node));
    temp = head;
    node* new = malloc(sizeof(node));
    if (new == NULL)
    {
        exit(1);
    }
    new->storage[0] = g.y;
    new->storage[1] = g.x;
    new->storage[2] = g.board[g.y][g.x];
    if (head == NULL)
    {
        head = new;
        return;
    }
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new;
}

void deleteNode(void)
{
    node* temp = malloc(sizeof(node));
    temp = head;
    node* end = malloc(sizeof(node));
    if (head->next == NULL)
    {
        free(head);
        head = NULL;
    }
    else
    {
        while (temp->next != NULL)
        {
            end = temp;
            temp = temp->next;
        }
        free(end->next);
        end->next = NULL;
    }
}
