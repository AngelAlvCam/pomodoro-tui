#include <string.h>
#include <panel.h>
#include <form.h>
#include <unistd.h>  // For usleep

int seconds_passed(int, int*);
int run_timer(int);
void run_alert(WINDOW*);
void clear_line(WINDOW*, int);
WINDOW* create_subwindow(WINDOW*, float);
void embed_form(WINDOW*, FORM*);
void print_middle(WINDOW*, int, char*);

/*
Function to clear a line in a given boxed window
*/
void clear_line(WINDOW* win, int line_number)
{
    wmove(win, line_number, 0);
    wclrtoeol(win);
    box(win, 0, 0);
}

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
This function embeds a form in the center of a window
*/
void embed_form(WINDOW* win, FORM* form)
{
    // Calculates dimension of the form
    int form_height, form_width;
    scale_form(form, &form_height, &form_width);

    // Calculates dimension of the window
    int win_height, win_width;
    getmaxyx(win, win_height, win_width);

    // Attach form to window
    int start_x = (win_width - form_width) / 2;
    int start_y = (win_height - form_height) / 2;
    set_form_win(form, win);
    set_form_sub(form, derwin(win, form_height, form_width, start_y, start_x));
}

/*
Function that prints a string in the middle of a given window, in different
positions: middle, top and bottom. The position is based on the value of the position
argument.
    position - real
    -1         Top of the window
    0          Center of the window
    1          Bottom of the window
*/
void print_middle(WINDOW* win, int position, char* message)
{
    // Cursor position in which the message starts
    int x_start, y_start;

    // Get window dimensions
    int win_width, win_height;
    getmaxyx(win, win_height, win_width);

    // Calculates vertical cursor position to print the message
    switch (position)
    {
    case -1:
        // print in the bottom of the grid (top of the window) 
        y_start = 1;
        break;
    
    case 0:
        // Print in middle line
        y_start = win_height / 2;
        break;

    case 1:
        // Print in the top of the grid (bottom of the window)
        y_start = win_height - 2;
        break;
    }

    // Calculates horizontal cursor position to print the message
    x_start = (win_width - strlen(message)) / 2;
    
    // Print message
    clear_line(win, y_start);
    mvwaddstr(win, y_start, x_start, message);
    // wrefresh(win);
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

    // Create form based on the fields
    my_form = new_form(field);
    // getmaxyx(stdscr, rows, cols);
    // scale_form(my_form, &rows, &cols);

    // Create window to store form
    // popup_window = newwin(rows + 4, cols + 4, 4, 4);
    popup_window = create_subwindow(progress_window, 0.5);
    print_middle(popup_window, -1, "Insert to confirm");
    keypad(popup_window, TRUE);

    // Attach form to window
    // set_form_win(my_form, popup_window);
    // set_form_sub(my_form, derwin(popup_window, rows, cols, 15, 5));
    embed_form(popup_window, my_form);

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
                    print_middle(popup_window, 1, "Try again...");
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
            if (col_trigger >= 1) 
            {
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

    // Check if it is necessary to run the alert
    if (counter_status)    
    {
        // Run alert in popup
        werase(popup_window); // Clean the previous popup content
        run_alert(popup_window);
    }

    // Delete panels
    del_panel(my_panel[0]);
    del_panel(my_panel[1]);

    // Delete popup window
    werase(popup_window);
    wrefresh(popup_window);
    delwin(popup_window);

    // Delete progress bar window
    werase(progress_window);
    wrefresh(progress_window);
    delwin(progress_window);


    return counter_status;
}

/*
Function that triggers an alarm in the standard screen
*/
void run_alert(WINDOW* win)
{
    wtimeout(win, 0); // Sets getch as non-blocking in wwin
    print_middle(win, 0, "Press q to continue!");
    int ch;
    while(1)
    {
        beep();
        ch = wgetch(win);
        if (ch == 'q')
        {
            break;
        }
    }
    wtimeout(win, -1);
}


