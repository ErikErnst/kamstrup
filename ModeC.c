// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include "config.h"
#include "optical_eye_utils.h"

#define MODE_DATA_READOUT '\0'
#define MODE_PROGRAMMING '\1'
#define MODE_MANUFACTURER_SPECIFIC '\6' // Write config

void send_receive(int optical_eye_fd, char const *message)
{
    write(optical_eye_fd, message, strlen(message));
    int twice = 0;
    while (1) {
        char c;
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (received == 1) show_char(c); else exit(1);
        if (c == '\n') {
            if (twice) break; else twice = 1;
        }
    }
}

int main(int argc, char *argv[])
{
    int optical_eye_fd = setup_optical_eye(DEVICE, BAUDRATE);
    char mode;
    char *explanation = "data readout";

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

    char message[] = "\006OZ \r\n";
    switch (mode) {
        case MODE_DATA_READOUT: message[3] = '0';
        case MODE_PROGRAMMING: message[3] = '1';
        case MODE_MANUFACTURER_SPECIFIC: message[3] = '6';
    }
    send_receive(optical_eye_fd, message);
    ioctl(optical_eye_fd, I_FLUSH, FLUSHW);
    return 0;
}
