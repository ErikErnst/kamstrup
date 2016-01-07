// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include "config.h"
#include "optical_eye_utils.h"

#define MESSAGE_LINE_COUNT 10

int main(int argc, char *argv[])
{
    char *device = DEVICE;
    int baudrate = BAUDRATE;
    if (argc > 3) {
        printf("Usage: %s [device [baudrate]]\n", argv[0]);
        exit(0);
    }
    if (argc > 1) {
        device = argv[1];
        printf("%s: using device %s\n", argv[0], device);
    }
    if (argc > 2) {
        baudrate = baudrate_of(argv[0], argv[2]);
        printf("%s: using baudrate %s\n", argv[0], argv[2]);
    }
    int line_count = 0;
    int optical_eye_fd = setup_optical_eye(device, baudrate, IS_7E1);
    write(optical_eye_fd, "/?!\r\n", 5);
    while (1) {
        char c;
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (received == 1) {
            if (c != '\r' && c != '\n') show_char(c);
        } else {
            fail("No data received from the meter; exiting");
        }
        ioctl(optical_eye_fd, I_FLUSH, FLUSHW);
        if (c == '\n') {
            putchar(c);
            if (line_count++ == MESSAGE_LINE_COUNT) break;
        }
    }
    return 0;
}
