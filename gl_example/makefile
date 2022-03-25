
all:abgr glvendor

abgr:abgr.c
	gcc -o abgr abgr.c -lGL -lglut -lGLU

glvendor:glvendor.c
	gcc -o $@ $^ -lGL -lglut -lGLU


clean:
	rm -rf abgr glvendor
