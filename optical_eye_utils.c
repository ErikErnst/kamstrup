// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include "optical_eye_utils.h"

void fail(char const *msg)
{
    perror(msg);
    exit(-1);
}

int setup_optical_eye(char const *optical_eye_device, int optical_eye_baudrate)
{
    int optical_eye_fd = open(optical_eye_device, O_RDWR);
    struct termios  config;

    if (optical_eye_fd < 0)
        fail("Could not open the optical eye device.");
    if (!isatty(optical_eye_fd))
        fail("Optical eye device is not a TTY.");
    if (tcgetattr(optical_eye_fd, &config) < 0)
        fail("Error getting current configuration of optical eye device.");

    // Specify no input processing.
    config.c_iflag &=
        ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    // Specify no output processing.
    config.c_oflag &= 0;
    // Specify no line processing.
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    // Specify character processing:
    // reset existing char size mask, specify even parity,
    // then force 7 bit char size and parity checking.
    config.c_cflag &= ~(CSIZE | PARODD);
    config.c_cflag |= CS7 | PARENB;
    // Specify that read can return a single byte; no inter-char timeout.
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;
    // Specify the baud rate.
    if (cfsetispeed(&config, optical_eye_baudrate) < 0 || 
        cfsetospeed(&config, optical_eye_baudrate) < 0) {
        fail("Could not set the desired baud rate.");
    }
    
    // Send the configuration to the device.
    if (tcsetattr(optical_eye_fd, TCSAFLUSH, &config) < 0) {
        fail("Could not apply configuration to optical eye device.");
    }

    return optical_eye_fd;
}

