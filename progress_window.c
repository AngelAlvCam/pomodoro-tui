#include <form.h> // Already includes ncurses
#include <unistd.h>  // For usleep
#include <pthread.h>
#include <stdlib.h>

// Global variable that determines if the counter should stop
int stop_loop = 0;

// Define mutex
pthread_mutex_t mutex;


// Save the contents of the window into the provided 2-dimensional array
void save_window_contents(WINDOW *win, char **window_contents, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            window_contents[i][j] = mvwinch(win, i, j);
        }
    }
}

// Restore the window contents from the provided 2-dimensional array
void restore_window_contents(WINDOW *win, char **window_contents, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mvwprintw(win, i, j, "%c", window_contents[i][j]);
        }
    }
    box(win, 0, 0);
    // wrefresh(win);
}


// Parallel function to manage the pop up window
void* popup_thread(void* vargp)
{
    int is_active = 1; // Flag variable to hide or show the pop up
    
    const char *filename = ".popup";
    int const popup_width = COLS / 2;
    int const popup_height = LINES / 4;
    WINDOW* popup = newwin(popup_height, popup_width, 1, popup_width / 2);
    box(popup, 0, 0);
    wtimeout(popup,0);
    mvwaddstr(popup, 1, 1, "Hello");
    mvwaddstr(popup, 2, 1, "world");
    // wrefresh(popup);

    // Memory area to store window contents
    char **window_contents = malloc(popup_height * sizeof(char *));
    for (int i = 0; i < popup_height; i++)
    {
        window_contents[i] = malloc(popup_width * sizeof(char));
    }
    save_window_contents(popup, window_contents, popup_height, popup_width);

    // Interaction loop
    int ch;
    while(ch != 'e')
    {
        pthread_mutex_lock(&mutex);
        int local_stop_loop = stop_loop;
        pthread_mutex_unlock(&mutex);
        if (local_stop_loop) 
        {
            break;
        }

        // If user presses q, print popup
        ch = wgetch(popup);
        if (ch == 'q')
        {
            if (is_active == 1) 
            {
                // Empty the screen
                save_window_contents(popup, window_contents, popup_height, popup_width);
                werase(popup);
                is_active = 0;
            } 
            else 
            {
                // Draw again
                restore_window_contents(popup, window_contents, popup_height, popup_width);
                is_active = 1;
            }
        } 
    }

    // Clears and delete the pop up window when the loop stops
    werase(popup);
    wrefresh(popup);
    delwin(popup);
    free(window_contents);

    // Locks the mutex to update the global variable to stop the counter in 
    // main thread
    pthread_mutex_lock(&mutex);
    stop_loop = 1;
    pthread_mutex_unlock(&mutex);
}

int main() {
    initscr();                // Start ncurses mode
    noecho();                 // Disable character echoing
    curs_set(0);              // Hide the cursor
    box(stdscr, 0, 0);  // Draw a border around the default window

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
    int seconds = minutes * 60;

    // Draw the brackets '[' and ']' for the progress bar: Initial print
    mvaddch(starty, startx - 1, '[');
    mvaddch(starty, startx + bar_width + 1, ']');
    refresh();
    
    // Start child thread for pop up window 
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, popup_thread, NULL);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Specify the grow ratio of the progress bar in seconds 
    float ratio = (float)bar_width / seconds;

    // Main loop to iterate over seconds
    int seconds_counter = 0, minutes_counter = 0;
    int current_col = 0;
    float accumulate = ratio;

    for (int second = 0; second <= seconds; second++) {
        // Locks the mutex, so only main thread can read
        pthread_mutex_lock(&mutex);
        int local_stop_loop = stop_loop;
        pthread_mutex_unlock(&mutex);
        if (local_stop_loop)
        {
            break;
        }

        // Format counter
        mvprintw(counter_starty, counter_startx, "%02d:%02d", minutes_counter, seconds_counter);

        // Increase accumulate for next iteration
        accumulate += ratio;
        
        // Check accumulate to see if progress can be updated in the bar
        if (accumulate >= 1) {
            // Append the next '=' character only if progress_width increases
            mvaddch(starty, startx + current_col, '=');

            // Restart accumulate to draw a char later
            accumulate = 0;

            // Increase col var to draw in the next index when it is needed
            current_col++;
        }

        // Update screen to print bar and counter
        refresh();

        // Update counter
        seconds_counter += 1;
        if (seconds_counter == 60) {
            seconds_counter = 0;
            minutes_counter += 1;
        }

        sleep(1);  // Pause for 1 second
    }

    pthread_mutex_lock(&mutex);
    if (!stop_loop)
    {
        stop_loop = 1;
    }
    pthread_mutex_unlock(&mutex);

    // Once the loop is broken (for any reason...) wait for the child thread to join
    pthread_join(thread_id, NULL);
    pthread_mutex_destroy(&mutex);
    beep();

    mvprintw(1, (cols / 2) - 10, "Press any key to exit.");
    refresh();

    getch();  // Wait until a key is pressed

    endwin();  // Clean up and restore terminal to normal

    return 0;
}