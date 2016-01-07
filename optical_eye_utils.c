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

int setup_optical_eye(char const *optical_eye_device, 
                      int optical_eye_baudrate,
                      int is_7e1)
{
    int optical_eye_fd = open(optical_eye_device, O_RDWR);
    struct termios  config;

    if (optical_eye_fd < 0)
        fail("Could not open the optical eye device");
    if (!isatty(optical_eye_fd))
        fail("Optical eye device is not a TTY");
    if (tcgetattr(optical_eye_fd, &config) < 0)
        fail("Error getting current configuration of optical eye device");

    // Specify no input processing.
    config.c_iflag &=
        ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);

    // Specify no output processing.
    config.c_oflag &= 0;

    // Specify no line processing.
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    // Specify character processing.
    if (is_7e1) {
        // reset existing char size mask, specify even parity,
        // then force 7 bit char size and parity checking.
        config.c_cflag &= ~(CSIZE | PARODD);
        config.c_cflag |= CS7 | PARENB;
    } else {
        // reset existing char size mask, and disable parity,
        // then force 8 bit char size and 2 stop bits.
        config.c_cflag &= ~(CSIZE | PARENB);
        config.c_cflag |= CS8 | CSTOPB;
    }

    // Specify that read can return a single byte; no inter-char timeout.
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    // Specify the baud rate.
    if (cfsetispeed(&config, optical_eye_baudrate) < 0 ||
        cfsetospeed(&config, optical_eye_baudrate) < 0) {
        fail("Could not set the desired baud rate");
    }
    
    // Send the configuration to the device.
    if (tcsetattr(optical_eye_fd, TCSAFLUSH, &config) < 0) {
        fail("Could not apply configuration to optical eye device");
    }

    return optical_eye_fd;
}

static char *hex_char = "0123456789abcdef";

void show_char(unsigned char c)
{
    char default_output[2] = {' ', '\0'};
    char hex_output[4] = {'#', ' ', ' ', '\0'};
    char *output;
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
        case '\012': output =  "[LF]"; break;
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
        default:
            if (c >= ' ' && c <= '\x7f') {
                default_output[0] = c;
                output = default_output;
            } else {
                int byte = (unsigned char)c;
                hex_output[1] = hex_char[byte >> 4];
                hex_output[2] = hex_char[byte & '\x0f'];
                output = hex_output;
            }
    }
    printf(output);
}

static char *baudrate_names[] = {
    "50", "75", "110", "134", "150", "200", "300", "600",
    "1200", "1800", "2400", "4800", "9600", "19200", "38400",
    NULL
};

static int baudrate_values[] = {
    B50, B75, B110, B134, B150, B200, B300, B600,
    B1200, B1800, B2400, B4800, B9600, B19200, B38400
};

int baudrate_of(char *self, char *arg) {
    int index = 0;
    while (baudrate_names[index]) {
        if (!strcmp(baudrate_names[index], arg)) {
            return baudrate_values[index];
        }
        index++;
    }
    fprintf(stderr, "%s: Unknown baudrate '%s', exiting.\n", self, arg);
    exit(-1);
}

// The Kamstrup 382Lx7 uses the "XMODEM" crc.
unsigned short crc16(unsigned char* data_p, unsigned char length)
{
    unsigned char x;
    unsigned short crc = 0x0000;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^
            ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

void optical_eye_write(int fd, unsigned char *request, int request_length)
{
    unsigned char *pc = request;
    int chars_written = 1;
    write(fd, request, 1); // Start request mark.
    while (chars_written < request_length - 1) {
        switch (request[chars_written]) {
            case 0x06: write(fd, "\x1b\xf9", 2); break;
            case 0x0d: write(fd, "\x1b\xf2", 2); break;
            case 0x1b: write(fd, "\x1b\xe4", 2); break;
            case 0x40: write(fd, "\x1b\xbf", 2); break;
            case 0x80: write(fd, "\x1b\x7f", 2); break;
            default: write(fd, request + chars_written, 1);
        }
        pc++;
        chars_written++;
    }
    write(fd, request + (request_length - 1), 1); // End request mark.
}

int descape_package(unsigned char *buffer, int length) {
    int index, offset = 0;
    for (index = 0; index < length; index++) {
        if (buffer[index] == 0x1b) {
            index++; offset++;
            switch (buffer[index]) {
                case 0xf9:
                case 0xf2:
                case 0xe4:
                case 0xbf:
                case 0x7f:
                    buffer[index - offset] = buffer[index] ^ '\xff';
                    break;
                default:
                    // Wrong escape?! We will "descape it" for now.
                    buffer[index - offset] = buffer[index] ^ '\xff';
            }
        } else {
            if (offset > 0) buffer[index - offset] = buffer[index];
        }
    }
    return length - offset;
}
