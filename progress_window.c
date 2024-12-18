#include <string.h>
#include <panel.h>
#include <form.h>
#include <unistd.h>  // For usleep

/* This function creates a window that fits exactly in the middle of 
a given parent window. */
WINDOW* create_subwindow(WINDOW* parent, float scale)
{
    int parent_start_x, parent_start_y;
    int parent_width, parent_height;

    getmaxyx(parent, parent_height, parent_width);
    getbegyx(parent, parent_start_y, parent_start_x);

    int child_height = (int)(parent_height * scale);
    int child_width = (int)(parent_width * scale);
    int child_start_y = (int)(parent_start_y + ((parent_height - child_height) / 2));
    int child_start_x = (int)(parent_start_x + ((parent_width - child_width) / 2));

    return newwin(child_height, child_width, child_start_y, child_start_x);
}

// int seconds_passed(int, int*);
// int run_timer(int);
// void run_alert();
// 
// int main() {
//     initscr();                // Start ncurses mode
//     noecho();                 // Disable character echoing
//     raw();
//     curs_set(0);              // Hide the cursor
//     timeout(0);               // Non-blocking input
//     box(stdscr, 0, 0);        // Draw a border around the default window
// 
//     // Call run timer here
//     if (run_timer(1))
//     {
//         mvprintw(1, 1, "Well done");
//         run_alert();
//     }
//     else
//     {
//         mvprintw(1, 1, "Good luck next time");
//     }
//     refresh();
// 
//     // Configure getch to be blocking in the default screen, 
//     timeout(-1);
// 
//     beep(); // Makes a sound in the terminal before quitting
//     getch();  // Wait until a key is pressed
// 
//     endwin();  // Clean up and restore terminal to normal
//     return 0;
// }

/*
target is a integer that refer to the expected amount of seconds to wait.
counter is a pointer to the memory address of a second counter.
*/
int seconds_passed(int target, int* counter)
{
    const int time_step = 100000; // In microseconds

    usleep(time_step);

    if (*counter > target * 1000000)
    {
        // Trigger and restart the counter
        *counter = 0;
        return TRUE;
    }
    else 
    {
        (*counter) += time_step;
        return FALSE;
    }
}

/*
Runs the timer and handles progress in a non-default window
*/
int run_timer(int minutes)
{
    /*
    Creation of progress-bar/counter window 
    */
    // WINDOW* progress_window = newwin(LINES - 10, COLS - 10, 5, 5);
    WINDOW* progress_window = create_subwindow(stdscr, 0.9);
    box(progress_window, 0, 0);
    wtimeout(progress_window, 0);

    // Define progress bar parameters
    int cols, rows;
    getmaxyx(progress_window, rows, cols);
    int bar_width = (int)(cols * 0.8);
    int bar_starty = rows / 2;
    int bar_startx = (cols - bar_width) / 2;

    // Define counter parameters; format is {00:00}, len = 5
    int counter_starty = bar_starty - 2;
    int counter_startx = (cols - 5) / 2;

    // Time parameters
    int total_seconds = minutes * 60;

    // Draw the brackets '[' and ']' for the progress bar in the default window
    mvwaddch(progress_window, bar_starty, bar_startx - 1, '[');
    mvwaddch(progress_window, bar_starty, bar_startx + bar_width, ']');
    // refresh();  // This only affects default window

    /* Creation of pop-up window with a form to quit */
    FIELD *field[2];
    FORM *my_form;
    WINDOW *popup_window;

    // Set up the field
    field[0] = new_field(1, 14, 0, 0, 0, 0);
    field[1] = NULL;
    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP);

    // Create form
    my_form = new_form(field);
    getmaxyx(stdscr, rows, cols);
    scale_form(my_form, &rows, &cols);

    // Create window to store form
    // popup_window = newwin(rows + 4, cols + 4, 4, 4);
    popup_window = create_subwindow(progress_window, 0.5);
    keypad(popup_window, TRUE);

    // Attach form to window
    set_form_win(my_form, popup_window);
    set_form_sub(my_form, derwin(popup_window, rows, cols, 2, 2));

    box(popup_window, 0, 0);
    wtimeout(popup_window, 0);
    post_form(my_form);
    //wrefresh(popup);

    /* 
    Setting the windows as panels 
    */
    PANEL *my_panel[2];
    int is_popup_active = FALSE;
    my_panel[0] = new_panel(progress_window);
    my_panel[1] = new_panel(popup_window);
    hide_panel(my_panel[1]);
    
    update_panels();
    doupdate();

    // Calculate bar increment ratio
    float bar_ratio = (float)bar_width / total_seconds;
    float col_trigger = bar_ratio;
    int current_col = 0;
    int counter_status = TRUE; 

    // Main loop
    // int second_counter = 0;
    int minutes_display = minutes, seconds_display = 0;
    int micro_accumulator = 0;
    int ch;
    while (total_seconds >= 0)
    {
        // Check if popup is active
        if (is_popup_active)
        {
            // if it is active, close with esc and put characters in form
            ch = wgetch(popup_window); // read input from popup window
            switch (ch)
            {
            // Case to quit the popup by pressing esc
            case 27:
                hide_panel(my_panel[1]);
                is_popup_active = FALSE;
                break;

            // Cases for form manipulation
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;

            case KEY_RIGHT:
                form_driver(my_form, REQ_RIGHT_CHAR);
                break;
            
                case KEY_BACKSPACE:
                form_driver(my_form, REQ_PREV_CHAR);
                form_driver(my_form, REQ_DEL_CHAR);
                break;

            case '\n':
                form_driver(my_form, REQ_NEXT_FIELD); // Change to NULL field to force sync
                if (strcmp(field_buffer(field[0], 0), "I want to quit") == 0)
                {
                    // Condition to break the loop
                    total_seconds = -1;
                    counter_status = FALSE;
                } 
                // else if -> run cmatrix window...
                else 
                {
                    mvwaddstr(popup_window,0,0,"Try again...");
                }
                form_driver(my_form, REQ_DEL_LINE);
                break;

            // Write the character in the form
            default:
                form_driver(my_form, ch);
                break;

            }
        }
        else
        {
            // else, if popup is not active... open it by pressing esc key
            ch = wgetch(progress_window); // read input from default window (stdscr)
            if (ch == 'q') 
            {
                show_panel(my_panel[1]);
                is_popup_active = TRUE;
            }
        }

        update_panels();
        doupdate();

        // Print counter
        mvwprintw(progress_window, counter_starty, counter_startx, "%02d:%02d", minutes_display, seconds_display);
        // refresh();

        // Update time and bar progress
        if (seconds_passed(1, &micro_accumulator))
        {
            total_seconds--;
            minutes_display = total_seconds / 60;
            seconds_display = total_seconds % 60;
            
            // Checks if the conditions to shift cursor are filled
            if (col_trigger >= 1) {
                mvwaddch(progress_window, bar_starty, bar_startx + current_col, '=');
                col_trigger = 0;
                current_col++;
            }
            col_trigger += bar_ratio;
        }
    }

    // Unpost form and free memory 
    unpost_form(my_form);
    free_form(my_form);
    free_field(field[0]);
    free_field(field[1]);

    del_panel(my_panel[0]);
    del_panel(my_panel[1]);

    // Delete popup window
    werase(popup_window);
    wrefresh(popup_window);
    delwin(popup_window);

    werase(progress_window);
    wrefresh(progress_window);
    delwin(progress_window);

    // del_panel(my_panel[0]);
    // del_panel(my_panel[1]);
    return counter_status;
}

/*
Function that triggers an alarm in the standard screen
*/
void run_alert()
{
    timeout(0); // Sets getch as non-blocking
    int ch;
    while(1)
    {
        beep();
        ch = getch();
        if (ch == 'q')
        {
            break;
        }
    }
    timeout(-1);
}

void clear_line(WINDOW* win, int line_number)
{
    wmove(win, line_number, 0);
    clrtoeol();
    box(win, 0, 0);
}

