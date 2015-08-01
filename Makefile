# Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
# source code is governed by a BSD-style license that can be found in
# the LICENSE file.

all: ModeA ModeC

ModeC: ModeC.o optical_eye_utils.o
	$(CC) -g -o ModeC optical_eye_utils.o ModeC.o

ModeA: ModeA.o optical_eye_utils.o
	$(CC) -g -o ModeA optical_eye_utils.o ModeA.o 

clean:
	rm *.o ModeA ModeC

%.o: %.c Makefile
	$(CC) -g -c $<

ModeA.o: optical_eye_utils.h config.h
optical_eye_utils.o: optical_eye_utils.h config.h

