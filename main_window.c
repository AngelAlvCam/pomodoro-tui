#include <string.h>
#include <ncurses.h>

int main()
{
    initscr();
    noecho();
    // raw();
    curs_set(0);
    timeout(-1); // Make it blocking 
    box(stdscr,0,0);
    keypad(stdscr, TRUE);

    // Display title
    char* title = "POMODORO TUI";
    mvaddstr(LINES / 2, (COLS - strlen(title)) / 2, title);
    refresh();

    // Minutes selection
    int minutes = 5; // Default and minimum
    while(1)
    {
        // Display the value:
        mvprintw(LINES / 2 + 2, (COLS - 5) / 2, "%02d:00", minutes);

        int ch = getch();
        switch (ch)
        {
        case KEY_LEFT:
            if (minutes > 5)
            {
                minutes--;
            }
            break;

        case KEY_RIGHT:
            if (minutes < 60)
            {
                minutes++;
            }
        
        default:
            // None
            break;
        }

    }


}