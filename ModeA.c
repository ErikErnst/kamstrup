#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include "setup_optical_eye.h"

#define OPTICAL_EYE_DEVICE "/dev/ttyUSB0"
#define OPTICAL_EYE_BAUD B9600

int main(int argc, char *argv[])
{
    int optical_eye_fd = setup_optical_eye();
    write(optical_eye_fd, "/?!\r\n", 5);
    while (1) {
        char c;
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (received == 1) printf("%c", c);
        else break;
    }
    return 0;
}
