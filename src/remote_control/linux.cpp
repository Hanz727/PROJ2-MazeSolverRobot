#include <clocale>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>

#define MAX_INPUT_SIZE 256

bool running = true;
WINDOW* input_window = NULL;
WINDOW* output_window = NULL;
int hCom = 0;

pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;

void create_windows() {
    setlocale(LC_ALL, "POSIX");

    initscr();            
    cbreak();   // Disable line buffering

    int height, width;
    getmaxyx(stdscr, height, width);

    input_window = newwin(3, width, height - 3, 0);
    output_window = newwin(height-3, width, 0, 0);

    scrollok(output_window, TRUE);
    keypad(input_window, FALSE); 
}

void print_safe(const char *format, ...) {
    pthread_mutex_lock(&write_mutex);

    va_list args;
    va_start(args, format);

    char formatted_string[1024]; 
    vsnprintf(formatted_string, sizeof(formatted_string), format, args);

    if (output_window)
        wprintw(output_window, "%s", formatted_string);

    wrefresh(output_window); 

    if (input_window)
        wrefresh(input_window); // return cursor back to input

    va_end(args);
    pthread_mutex_unlock(&write_mutex);
}

void refresh_input_window() {
    pthread_mutex_lock(&write_mutex);
    
    wclear(input_window);
    box(input_window, 0, 0);
    mvwprintw(input_window, 1, 1, "Type: ");
    wrefresh(input_window);

    pthread_mutex_unlock(&write_mutex);
}

void check_device(const char *device) {
    char cmd[256];
    FILE *fp;

    // Command to get udev information about the device
    snprintf(cmd, sizeof(cmd), "udevadm info --query=all --name=%s", device);

    // Execute the command and capture the output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("Failed to run udevadm");
        return;
    }

    char line[256];
    int arduino = 0;
    int bt = 0;

    // Check for keywords indicating Arduino or Bluetooth devices
    while (fgets(line, sizeof(line), fp) != NULL) {
        //print_safe(line);
        if (strstr(line, "Arduino") ) {
            arduino = 1;
        }

        if (strstr(line, "bluetooth")) {
            bt = 1;
        }
    }

    if (arduino) {
        print_safe("[Arduino] Connected device: %s\n", device);
    }

    if (bt) {
        print_safe("[BLUETOOTH] Connected device: %s\n", device);
    }

    fclose(fp);
}

void list_com_ports() {
    DIR *dir = opendir("/dev");
    if (dir == NULL) {
        perror("Unable to open /dev directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if the filename matches a serial port pattern
        if (strncmp(entry->d_name, "ttyS", 4) == 0 ||    // Standard serial ports
            strncmp(entry->d_name, "ttyUSB", 6) == 0 ||   // USB serial devices
            strncmp(entry->d_name, "ttyACM", 6) == 0 ||
            strncmp(entry->d_name, "rfcomm", 6) == 0) {   
            
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);
            check_device(device_path);
        }
    }

    // Close the directory
    closedir(dir);
}

int open_serial_port(const char* port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        print_safe("Error opening com port");
        _exit(1);
    }

    return fd;
}


void configure_serial_port(int fd) {
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    // Get the current configuration of the serial port
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting port attributes");
        _exit(1);
    }

    // Set the baud rate (9600 in this case)
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // 8 data bits, no parity, 1 stop bit
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;  // Clear data bits mask
    tty.c_cflag |= CS8;     // 8 data bits

    // Set raw input/output mode (disable software flow control)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control

    // Set the new attributes
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting port attributes");
        _exit(1);
    }
}

void read_full_message(int fd) {
    char buf[1024];
    int total_bytes = 0;
    int n;

    // Loop until a full message is received
    while (1) {
        n = read(fd, buf + total_bytes, sizeof(buf) - total_bytes);
        if (n < 0) {
            print_safe("Error reading from serial port");
            return;
        } 

        total_bytes += n;

        if (buf[total_bytes - 1] == '\n') {
            break;
        }

        if (total_bytes >= sizeof(buf) - 1) {
            buf[total_bytes] = '\0';
            print_safe("Message too large: %s\n", buf);
            return;
        }
    }

    // Null-terminate the string and process the complete message
    buf[total_bytes] = '\0';
    if (strlen(buf) > 1)
        print_safe("<<: %s", buf);
}

void write_to_port(int fd, const char *data) {
    int n = write(fd, data, strlen(data));
    if (n < 0) {
        print_safe("Error writing to serial port");
    }
}

void* recv_loop(void* arg) {
    while (running) {
        //print_safe("hi\n");
        read_full_message(hCom);
    }

    return NULL;
}

void input_loop() {
    while (1) {
        char input[MAX_INPUT_SIZE]{0};
        wrefresh(output_window);
        refresh_input_window();

        wgetnstr(input_window, input, MAX_INPUT_SIZE - 1);
        strncat(input, "\n", MAX_INPUT_SIZE - strlen(input) - 1);

        print_safe(">>: %s", input);

        write_to_port(hCom, input);

        if (strcmp(input, "/exit") == 0) break; // Exit command
        if (strcmp(input, "/ls") == 0) list_com_ports(); // Exit command
    }
    
    running = false;
}

int main() {
    create_windows();

    hCom = open_serial_port("/dev/rfcomm0");
    configure_serial_port(hCom); 
    
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, recv_loop, NULL); 

    input_loop(); 

    pthread_join(recv_thread, NULL);

    endwin();
    close(hCom);
}

