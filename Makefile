# Copyright (c) 2015, Erik Ernst. All rights reserved. Use of this
# source code is governed by a BSD-style license that can be found in
# the LICENSE file.

ModeA: ModeA.o optical_eye_utils.o
	$(CC) -o ModeA ModeA.o setup_optical_eye.o

clean:
	rm *.o

%.o: %.c Makefile
	$(CC) -c $<

ModeA.o: optical_eye_utils.h config.h
optical_eye_utils.o: optical_eye_utils.h config.h

