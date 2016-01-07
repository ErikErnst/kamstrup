// Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
// source code is governed by a BSD-style license that can be found in
// the LICENSE file.

#define IS_7E1 1
#define IS_8N2 0

#define BUFFER_LENGTH 8192

int setup_optical_eye(char const *optical_eye_device, 
                      int optical_eye_baudrate,
                      int is_7e1);

void show_char(unsigned char c);

int baudrate_of(char *self, char *arg);

unsigned short crc16(unsigned char* data_p, unsigned char length);

void optical_eye_write(int fd, unsigned char *request, int request_length);

int descape_package(unsigned char *buffer, int length);

