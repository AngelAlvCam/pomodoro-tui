#include <string.h>
#include <form.h>
#include <ncurses.h>
#include <unistd.h>  // For usleep

int main() {
    initscr();                // Start ncurses mode
    noecho();                 // Disable character echoing
    curs_set(0);              // Hide the cursor
    timeout(0);               // Non-blocking input
    box(stdscr, 0, 0);        // Draw a border around the default window

    int rows, cols;
    getmaxyx(stdscr, rows, cols);  // Get screen dimensions

    // Define progress bar parameters
    int bar_width = 60;
    int starty = rows / 2;
    int startx = (cols - bar_width) / 2;

    // Define counter parameters; format is {00:00}, len = 5
    int counter_starty = starty - 2;
    int counter_startx = (cols - 5) / 2;

    // Time parameters
    int minutes = 1;
    int total_seconds = minutes * 60;

    // Draw the brackets '[' and ']' for the progress bar in the default window
    mvaddch(starty, startx - 1, '[');
    mvaddch(starty, startx + bar_width, ']');
    refresh();  // This only affects default window

    // Popup window parameters and form configuration
    FIELD *field[2];
    FORM *my_form;
    WINDOW *popup;
    
    field[0] = new_field(1, 14, 0, 0, 0, 0);
    field[1] = NULL;

    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP);

    my_form = new_form(field);

    scale_form(my_form, &rows, &cols);

    popup = newwin(rows + 4, cols + 4, 4, 4);
    keypad(popup, TRUE);

    set_form_win(my_form, popup);
    set_form_sub(my_form, derwin(popup, rows, cols, 2, 2));

    box(popup, 0, 0);

    wtimeout(popup, 0);
    post_form(my_form);
    wrefresh(popup);

    // Calculate bar increment ratio
    float bar_ratio = (float)bar_width / total_seconds;
    float col_trigger = bar_ratio;
    int current_col = 0;

    // Main loop
    int second_counter = 0;
    int minutes_display = 0, seconds_display = 0;
    int micro_accumulator = 0;
    int ch;
    while (second_counter <= total_seconds) {

        // Handle popup window interaction
        ch = wgetch(popup);
        
        switch(ch)
        {
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_RIGHT_CHAR);
                break;
            //case for del 
            case '\n':
                // Change to NULL field to force sync
                form_driver(my_form, REQ_NEXT_FIELD);
                if (strcmp(field_buffer(field[0], 0), "I want to quit") == 0)
                {
                    total_seconds = -1;
                } 
                else 
                {
                    mvwaddstr(popup,0,0,"Try again...");
                }
                form_driver(my_form, REQ_DEL_LINE);
                break;
            default:
                form_driver(my_form, ch);
                break;
        }

        // Print counter
        mvprintw(counter_starty, counter_startx, "%02d:%02d", minutes_display, seconds_display);
        refresh();

        // Update time and bar progress
        usleep(100000);  // Sleep for 100 ms
        micro_accumulator += 100000;

        if (micro_accumulator >= 1000000) {  // 1 second elapsed
            micro_accumulator = 0;
            second_counter++;
        
            minutes_display = second_counter / 60;
            seconds_display = second_counter % 60;

            // Checks if the conditions to shift cursor are filled
            if (col_trigger >= 1) {
                mvaddch(starty, startx + current_col, '=');
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

    // Configure getch to be blocking in the default screen, 
    timeout(-1);

    // Final message printed in the default window
    mvprintw(1, (cols / 2) - 10, "Press any key to exit.");
    refresh();

    beep(); // Makes a sound in the terminal before quitting
    getch();  // Wait until a key is pressed

    endwin();  // Clean up and restore terminal to normal
    return 0;
}
