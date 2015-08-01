
ModeA: ModeA.c setup_optical_eye.c setup_optical_eye.h
	gcc -c ModeA.c
	gcc -c setup_optical_eye.c
	gcc -o ModeA ModeA.o setup_optical_eye.o

clean:
	rm *.o

