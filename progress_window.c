#include <string.h>
// ncurses is already included in panel.h and form.h
#include <panel.h>
#include <form.h>
#include <unistd.h>  // For usleep

typedef struct _PANEL_DATA {
	int hide;	/* TRUE if panel is hidden */
}PANEL_DATA;

int main() {
    initscr();                // Start ncurses mode
    noecho();                 // Disable character echoing
    raw();
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

    // Set up the field
    field[0] = new_field(1, 14, 0, 0, 0, 0);
    field[1] = NULL;
    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP);

    // Create form
    my_form = new_form(field);
    scale_form(my_form, &rows, &cols);

    // Create window to store form
    popup = newwin(rows + 4, cols + 4, 4, 4);
    keypad(popup, TRUE);

    // Attach form to window
    set_form_win(my_form, popup);
    set_form_sub(my_form, derwin(popup, rows, cols, 2, 2));

    box(popup, 0, 0);
    wtimeout(popup, 0);
    post_form(my_form);
    //wrefresh(popup);

    // PANELs config
    PANEL *my_panels[2];
    PANEL_DATA panel_datas[2];
    PANEL_DATA *temp;

    my_panels[0] = new_panel(stdscr);
    my_panels[1] = new_panel(popup);
    
    panel_datas[0].hide = FALSE;
    panel_datas[1].hide = TRUE;

    set_panel_userptr(my_panels[0], &panel_datas[0]);
    set_panel_userptr(my_panels[1], &panel_datas[1]);

    update_panels();
    doupdate();

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
        // Check if popup is active
        temp = (PANEL_DATA*)panel_userptr(my_panels[1]);
        if (temp->hide == FALSE)
        {
            // if it is active, close with esc and put characters in form
            ch = wgetch(popup); // read input from popup window
            switch (ch)
            {
            // Case to quit the popup by pressing esc
            case 27:
                hide_panel(my_panels[1]);
                temp->hide = TRUE;
                break;

            // Cases for form manipulation
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_RIGHT_CHAR);
                break;
            case '\n':
                form_driver(my_form, REQ_NEXT_FIELD); // Change to NULL field to force sync
                if (strcmp(field_buffer(field[0], 0), "I want to quit") == 0)
                {
                    // Condition to break the loop
                    total_seconds = -1;
                } 
                else 
                {
                    mvwaddstr(popup,0,0,"Try again...");
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
            ch = getch(); // read input from default window (stdscr)
            if (ch == 27) 
            {
                show_panel(my_panels[1]);
                temp->hide = FALSE;
            }
        }

        update_panels();
        doupdate();

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

    // Delete popup window
    werase(popup);
    wrefresh(popup);
    delwin(popup);

    // Configure getch to be blocking in the default screen, 
    timeout(-1);

    // Final message printed in the default window
    mvprintw(1, 1, "Press any key to exit.");
    refresh();

    beep(); // Makes a sound in the terminal before quitting
    getch();  // Wait until a key is pressed

    endwin();  // Clean up and restore terminal to normal
    return 0;
}
