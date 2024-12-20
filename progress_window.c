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

const char* ascii_digits[10][5] = {
        { // Symbol 0
            "000000",
            "00  00",
            "00  00",
            "00  00",
            "000000"
        },
        { // Symbol 1
            "  11  ",
            "1111  ",
            "  11  ",
            "  11  ",
            "111111"
        },
        { // Symbol 2
            "222222",
            "22  22",
            "   22 ",
            "  22  ",
            "222222"
        },
        { // Symbol 3
            "333333",
            "33  33",
            "   33 ",
            "33  33",
            "333333"
        },
        { // Symbol 4
            "44  44",
            "44  44",
            "444444",
            "    44",
            "    44"
        },
        { // Symbol 5
            "555555",
            "55    ",
            "555555",
            "    55",
            "555555"
        },
        { // Symbol 6
            "666666",
            "66    ",
            "666666",
            "66  66",
            "666666"
        },
        { // Symbol 7
            "777777",
            "   77 ",
            "  77  ",
            " 77   ",
            "77    "
        },
        { // Symbol 8
            "888888",
            "88  88",
            "888888",
            "88  88",
            "888888"
        },
        { // Symbol 9
            "999999",
            "99  99",
            "999999",
            "    99",
            "    99"
        }
    }; 


void render_time(WINDOW* win, int minutes, int seconds)
{
    // Calculate starting position
    int width, height;
    getmaxyx(win, height, width); 
    int startx = (width - 27) / 2;
    int starty = (height - 6) / 2;

    // check which digits to render
    int minutes_dec = minutes / 10;
    int minutes_unit = minutes % 10;
    int seconds_dec = seconds / 10;
    int seconds_unit = seconds % 10;

    int digits[4] = {minutes_dec, minutes_unit, seconds_dec, seconds_unit};

    // digits are arrays of 6 lines of 5 characters
    for (int i = 0; i < 4; i++)
    {
        int current_digit = digits[i];
        for (int j = 0; j < 5; j++)
        {
            mvwaddstr(win, starty + j, startx, ascii_digits[current_digit][j]);
        }
        startx = startx + 7;
    }
}

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
    WINDOW* progress_window = create_subwindow(stdscr, 0.9);
    box(progress_window, 0, 0);
    wtimeout(progress_window, 0);

    // Define progress bar parameters
    int cols, rows;
    getmaxyx(progress_window, rows, cols);

    // Time parameters
    int total_seconds = minutes * 60;

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

    // Create window to store form
    popup_window = create_subwindow(progress_window, 0.5);
    print_middle(popup_window, -1, "Insert to confirm");
    keypad(popup_window, TRUE);

    // Attach form to window
    embed_form(popup_window, my_form);

    box(popup_window, 0, 0);
    wtimeout(popup_window, 0);
    post_form(my_form);

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
        render_time(progress_window, minutes_display, seconds_display);

        // Update time and bar progress
        if (seconds_passed(1, &micro_accumulator))
        {
            total_seconds--;
            minutes_display = total_seconds / 60;
            seconds_display = total_seconds % 60;
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


