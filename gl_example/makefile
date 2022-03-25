
all:abgr glvendor eglvendor glxvendor

abgr:abgr.c
	gcc -o abgr abgr.c -lGL -lglut -lGLU

glvendor:glvendor.c
	gcc -o $@ $^ -lGL -lglut -lGLU -lGLEW

glxvendor:glxvendor.c
	gcc -o $@ $^ -lGL -lglut -lGLU -lGLEW

eglvendor:eglvendor.c
	gcc -o $@ $^ -lGL -lglut -lGLU -lGLEW -lX11 -lEGL

clean:
	rm -rf abgr glvendor
