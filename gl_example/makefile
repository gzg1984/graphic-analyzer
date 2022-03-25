
all:abgr glvendor

abgr:abgr.c
	gcc -o abgr abgr.c -lGL -lglut -lGLU

glvendor:glvendor.c
	gcc -o $@ $^ -lGL -lglut -lGLU -lGLEW


clean:
	rm -rf abgr glvendor
