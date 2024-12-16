#include <string.h>
#include <ncurses.h>

int timer(int);

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

    // Message
    char* message = NULL;

    // Minutes selection
    int minutes = 5; // Default and minimum
    while(1)
    {
        // Display the chosen value 
        mvprintw(LINES / 2 + 2, (COLS - 5) / 2, "%02d:00", minutes);

        // Instruction to set message if needed        
        if (message != NULL)
        {
            mvaddstr(LINES /2 - 2, (COLS - strlen(message)) / 2, message);
        }
        else
        {
            // Delete message after 5 seconds
        }

        refresh();

        int ch = getch();
        if (ch == 27)
        {
            break;
        }
        else
        {
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
                break;

            case '\n':
                // Start running app!
                if (timer(minutes))
                {
                    message = "Well done!";
                }
                else
                {
                    message = "What a shame... better luck next time";
                }
                break;
        
            default:
                // None
                break;
            }
        }
    }
}

/*
This function runs the pomodoro timer given a value in minutes
*/
int timer(int minutes)
{
    
    return TRUE;
}