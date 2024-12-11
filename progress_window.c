#include <ncurses.h>
#include <unistd.h>  // For usleep

int main() {
    initscr();                // Start ncurses mode
    noecho();                 // Disable character echoing
    curs_set(0);              // Hide the cursor
    timeout(0);               // Non-blocking input
    box(stdscr, 0, 0);        // Draw a border around the window

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

    // Draw the brackets '[' and ']' for the progress bar
    mvaddch(starty, startx - 1, '[');
    mvaddch(starty, startx + bar_width, ']');
    refresh();

    // Popup window parameters
    int popup_active = 0;
    int popup_height = 10;
    int popup_width = 30;
    int popup_starty = (rows - popup_height + 15) / 2;
    int popup_startx = (cols - popup_width) / 2;
    WINDOW *popup = NULL;

    // Calculate bar increment ratio
    float bar_ratio = (float)bar_width / total_seconds;
    float col_trigger = bar_ratio;
    int current_col = 0;

    // Main loop
    int second_counter = 0;
    int minutes_display = 0, seconds_display = 0;
    int micro_accumulator = 0;
    while (second_counter <= total_seconds) {

        // Handle popup window interaction
        int ch = getch();
        if (ch == 'q') {
            if (!popup_active) {
                // Activate popup
                popup_active = 1;
                popup = newwin(popup_height, popup_width, popup_starty, popup_startx);
                box(popup, 0, 0);
                mvwprintw(popup, 2, 4, "Popup Window");
                mvwprintw(popup, 5, 4, "Press 'q' to close");
                wrefresh(popup);
            } else {
                // Deactivate popup
                popup_active = 0;
                werase(popup);
                wrefresh(popup);
                delwin(popup);
            }
        } else if (ch == 'e') {
            break;  // Exit the program
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

    // Configure getch to be blocking
    timeout(-1);

    beep();

    // Final message
    mvprintw(1, (cols / 2) - 10, "Press any key to exit.");
    refresh();
    getch();  // Wait until a key is pressed

    endwin();  // Clean up and restore terminal to normal
    return 0;
}
