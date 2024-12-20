#include <string.h>
#include <ncurses.h>
#include "progress_window.h"

int set_timer(int*, int, int, int);

int main()
{
    initscr();
    noecho();
    // raw();
    curs_set(0);
    timeout(-1); // Make it blocking 
    box(stdscr,0,0);
    keypad(stdscr, TRUE);

    // time configuration
    int min_minutes, max_minutes, step;

    // Boolean to check if the timer is in focus or rest mode
    int is_focus = TRUE;

    // Message
    char* message = NULL;

    // Minutes selection
    int task_minutes = 25;
    int rest_minutes = 5;    

    while(1)
    {
        // Set task timer
        print_middle(stdscr, -1, "TASK MODE");
        int status = set_timer(&task_minutes, 5, 60, 5);
        
        // quit app
        if (status == 2)
        {
            break;
        } 
        
        switch(status)
        {
            // If the task timer ran without quitting, change to rest mode
            case 0:
                print_middle(stdscr, -1, "REST MODE");
                print_middle(stdscr, 1, "Well done! Now take a rest");
                set_timer(&rest_minutes, 1, 30, 1);
                print_middle(stdscr, 1, "Great! Lets keep working");
                break;

            // If the task timer got aborted, keep the task mode
            case 1:
                print_middle(stdscr, 1, "Good luck the next time");
                break;
        }
    }

    endwin();
    return 0;
}

/*
Function to configure a timer, for task mode or rest mode
*/
int set_timer(int* minutes, int min_minutes, int max_minutes, int step)
{
    int has_finished = FALSE;
    while(1)
    {
        // Display the chosen value 
        render_time(stdscr, *minutes, 0);
        
        int ch = getch();
        if (ch == 'q' || ch == 27)
        {
            return 2;
        }
        else if (ch == '\n')
        {
            if (run_timer(1))
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            switch (ch)
            {
            case KEY_LEFT:
                if (*minutes > min_minutes)
                {
                    *minutes -= step;
                }
                break;

            case KEY_RIGHT:
                if (*minutes < max_minutes)
                {
                    *minutes += step;
                }
                break;
            }
        }    
    }
}