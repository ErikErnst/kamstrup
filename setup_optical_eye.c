#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>

#define OPTICAL_EYE_DEVICE "/dev/ttyUSB0"
#define OPTICAL_EYE_BAUD B9600

void fail(char const *msg)
{
    perror(msg);
    exit(-1);
}

int setup_optical_eye()
{
    int optical_eye_fd = open(OPTICAL_EYE_DEVICE, O_RDWR);
    struct termios  config;

    if (optical_eye_fd < 0)
        fail("Cannot open optical eye device.");
    if (!isatty(optical_eye_fd))
        fail("The optical eye device is not TTY device.");

    // Get current configuration.
    if (tcgetattr(optical_eye_fd, &config) < 0)
        fail("The optical eye device is not TTY device.");

    // Specify no input processing.
    config.c_iflag &=
        ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);

    // Specify no output processing.
    config.c_oflag &= 0;

    // Specify no line processing.
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    // Specify character processing:
    // reset existing char size mask, specify even parity,
    // then force 7 bit char size, parity checking
    config.c_cflag &= ~(CSIZE | PARODD);
    config.c_cflag |= CS7 | PARENB;

    // Read can return a single byte; no timeout between characters.
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    // Specify the baud rate.
    if (cfsetispeed(&config, OPTICAL_EYE_BAUD) < 0 || 
        cfsetospeed(&config, OPTICAL_EYE_BAUD) < 0) {
        fail("Could not set the desired baud rate.");
    }
    
    // Apply the configuration
    if (tcsetattr(optical_eye_fd, TCSAFLUSH, &config) < 0) {
        fail("Could not apply configuration to optical eye device.");
    }

    return optical_eye_fd;
}
