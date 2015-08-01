// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
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

void show_char(char c)
{
    char default_output[2] = " "; // Ensure '\0' at entry 1.
    char *output = default_output;
    switch (c) {
        case '\000': output = "[NUL]"; break;
        case '\001': output = "[SOH]"; break;
        case '\002': output = "[STX]"; break;
        case '\003': output = "[ETX]"; break;
        case '\004': output = "[EOT]"; break;
        case '\005': output = "[ENQ]"; break;
        case '\006': output = "[ACK]"; break;
        case '\007': output = "[BEL]"; break;
        case '\010': output =  "[BS]"; break;
        case '\011': output = "[TAB]"; break;
        case '\012': output =  "[LF]\n"; break;
        case '\013': output =  "[VT]"; break;
        case '\014': output =  "[FF]"; break;
        case '\015': output =  "[CR]"; break;
        case '\016': output =  "[SO]"; break;
        case '\017': output =  "[SI]"; break;
        case '\020': output = "[DLE]"; break;
        case '\021': output = "[DC1]"; break;
        case '\022': output = "[DC2]"; break;
        case '\023': output = "[DC3]"; break;
        case '\024': output = "[DC4]"; break;
        case '\025': output = "[NAK]"; break;
        case '\026': output = "[SYN]"; break;
        case '\027': output = "[ETB]"; break;
        case '\030': output = "[CAN]"; break;
        case '\031': output =  "[EM]"; break;
        case '\032': output = "[SUB]"; break;
        case '\033': output = "[ESC]"; break;
        case '\034': output =  "[FS]"; break;
        case '\035': output =  "[GS]"; break;
        case '\036': output =  "[RS]"; break;
        case '\037': output =  "[US]"; break;
        case '\x7F': output = "[DEL]"; break;
        default: default_output[0] = c;
    }
    printf("%s",output);
}
