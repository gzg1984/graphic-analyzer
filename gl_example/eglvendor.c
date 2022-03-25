#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <stdio.h> 


static Display *display;
static int screen;
static Window win = 0;

static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLSurface eglSurface = EGL_NO_SURFACE;
static EGLContext eglContext = EGL_NO_CONTEXT;

// glut window handle
static int window;

int main(int ac,char**av)
{
    display = XOpenDisplay(NULL);

    if (!display)
    {
        printf("\nError opening X display\n");
        return -1;
    }

    screen = DefaultScreen(display);
    printf("screen is %d\n",screen);

    eglDisplay = eglGetDisplay(display);

    if (eglDisplay == EGL_NO_DISPLAY)
    {
       printf( "\nEGL : failed to obtain display\n");
        return -1;
    }

    if (!eglInitialize(eglDisplay, 0, 0))
    {
        printf( "\nEGL : failed to initialize\n");
        return -1;
    }
}