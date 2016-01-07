// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include "config.h"
#include "optical_eye_utils.h"

#define MODE_DATA_READOUT 0
#define MODE_PROGRAMMING 1
#define MODE_MANUFACTURER_SPECIFIC 6 // Write config

void flush_optical_eye(int optical_eye_fd)
{
    ioctl(optical_eye_fd, I_FLUSH, FLUSHW);
}

void send_receive(int optical_eye_fd, char const *message)
{
    write(optical_eye_fd, message, strlen(message));
    flush_optical_eye(optical_eye_fd);
    int twice = 0;
    while (1) {
        char c;
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (received == 1) show_char(c); else exit(1);
        // if (c == '\n') {
        //    if (twice) break; else twice = 1;
        // }
        int stdin_read_status = read(stdin, &c, sizeof(c));
        if (stdin_read_status >= 0) {
            printf("{Leaving}");
            break;
        } else {
            printf("{%d,%c}", stdin_read_status, c);
        }
    }
}

int main(int argc, char *argv[])
{
    int optical_eye_fd = setup_optical_eye(DEVICE, BAUDRATE);
    int mode;
    char *explanation = "data readout";

    // Set up stdin to enable manuel control of the communication:
    // Will take next step when a key has been pressed on the 
    // keyboard.
    fcntl(F_SETFL, O_NONBLOCK);

    if (argc == 1) mode = MODE_DATA_READOUT;
    else {
        mode = (char)atoi(argv[1]);
        if (mode == MODE_DATA_READOUT)
            /* do nothing */;
        else if (mode == MODE_PROGRAMMING) 
            explanation = "programming";
        else if (mode == MODE_MANUFACTURER_SPECIFIC)
            explanation = "manufacturer specific";
        else 
            explanation = "unknown";
    }
    printf("Setting mode to %d (%s)\n", (int)mode, explanation);

    send_receive(optical_eye_fd, "/?!\r\n");

    char message[] = "\00605 \r\n";
    switch (mode) {
        case MODE_DATA_READOUT: message[3] = '0'; break;
        case MODE_PROGRAMMING: message[3] = '1'; break;
        case MODE_MANUFACTURER_SPECIFIC: message[3] = '6'; break;
        default: message[3] = (char)(mode + '0');
    }
    send_receive(optical_eye_fd, message);
    return 0;
}
