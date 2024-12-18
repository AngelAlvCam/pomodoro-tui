#include <ncurses.h>
int seconds_passed(int, int*);
int run_timer(int);
void run_alert();
void clear_line(WINDOW*, int);
WINDOW* create_subwindow(WINDOW*);
void print_middle(WINDOW*, int, char*);