# Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
# source code is governed by a BSD-style license that can be found in
# the LICENSE file.

all: iec1107 heartbeat readvar recentload

iec1107: iec1107.o optical_eye_utils.o
	$(CC) -g -o iec1107 optical_eye_utils.o iec1107.o 

heartbeat: heartbeat.o optical_eye_utils.o
	$(CC) -g -o heartbeat optical_eye_utils.o heartbeat.o

recentload: recentload.o optical_eye_utils.o
	$(CC) -g -o recentload optical_eye_utils.o recentload.o

readvar: readvar.o optical_eye_utils.o
	$(CC) -g -o readvar optical_eye_utils.o readvar.o -lm

clean:
	rm *.o iec1107 heartbeat recentload readvar

%.o: %.c Makefile
	$(CC) -g -c $<

iec1107.o: optical_eye_utils.h config.h
heartbeat.o: optical_eye_utils.h config.h
optical_eye_utils.o: optical_eye_utils.h config.h

