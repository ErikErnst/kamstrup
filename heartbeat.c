// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include "config.h"
#include "optical_eye_utils.h"

static char heartbeat_message[] = {
    '\x80', '\x3f', '\x10', '\x01', '\x04',
    '\x1b', '\xf9', '\xe9', '\x82', '\x0d'
};
static int heartbeat_message_length = 10;

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
    int optical_eye_fd = setup_optical_eye(device, baudrate, IS_8N2);

    while (1) {
        write(optical_eye_fd, heartbeat_message, heartbeat_message_length);
        int received_total = 0;
        char c = '\0';
        while (received_total < 25 || c != '\r') {
            int received = read(optical_eye_fd, &c, sizeof(c));
            if (received == 1) {
                received_total++;
                show_char(c);
                if (c == '\r') printf("\n");
            }
            ioctl(optical_eye_fd, I_FLUSH, FLUSHW);
        }
        sleep(1);
    }
    return 0;
}
