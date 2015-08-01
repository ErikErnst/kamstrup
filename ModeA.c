// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <stdio.h>
#include "config.h"
#include "optical_eye_utils.h"

int main(int argc, char *argv[])
{
    int optical_eye_fd = setup_optical_eye(DEVICE, BAUDRATE);
    write(optical_eye_fd, "/?!\r\n", 5);
    while (1) {
        char c;
        int received = read(optical_eye_fd, &c, sizeof(c));
        if (received == 1) printf("%c", c);
        else break;
    }
    return 0;
}
