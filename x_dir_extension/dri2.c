/* TODO:
 * get current window ID
 * show it
 * run other program 
 * and re-parent to this one */

/* This example:
 *  - creates X11 container window;
 *  - executes child process (container window id is appended to argv);
 *  - handles child window requests;
 *  - propagates container window events to child window.
 *
 * Usage:
 *   gcc -o xlib_container xlib_container.c -lX11
 *   ./xlib_container xterm -into
 *   ./xlib_container mplayer video.mp4 -wid
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/signal.h>
#include <sys/wait.h>

#define WIDTH   400
#define HEIGHT  200

static const char *event_names[] = {
   "",
   "",
   "KeyPress",
   "KeyRelease",
   "ButtonPress",
   "ButtonRelease",
   "MotionNotify",
   "EnterNotify",
   "LeaveNotify",
   "FocusIn",
   "FocusOut",
   "KeymapNotify",
   "Expose",
   "GraphicsExpose",
   "NoExpose",
   "VisibilityNotify",
   "CreateNotify",
   "DestroyNotify",
   "UnmapNotify",
   "MapNotify",
   "MapRequest",
   "ReparentNotify",
   "ConfigureNotify",
   "ConfigureRequest",
   "GravityNotify",
   "ResizeRequest",
   "CirculateNotify",
   "CirculateRequest",
   "PropertyNotify",
   "SelectionClear",
   "SelectionRequest",
   "SelectionNotify",
   "ColormapNotify",
   "ClientMessage",
   "MappingNotify"
};


struct windowWithDeletePackage
{
        Window window;
        Atom wm_delete;
};

void createContainerWindowWithVMDelete(struct windowWithDeletePackage *new_window_package, Display* display,int screen)
{
    Colormap colormap = DefaultColormap(display, screen);

    XColor blue;
    XAllocNamedColor(display, colormap, "blue", &blue, &blue);

    // Initialize container window attributes
    XSetWindowAttributes attrs;

    attrs.event_mask
        = SubstructureRedirectMask // handle child window requests      (MapRequest)
        | SubstructureNotifyMask   // handle child window notifications (DestroyNotify)
        | StructureNotifyMask      // handle container notifications    (ConfigureNotify)
        | ExposureMask             // handle container redraw           (Expose)
        ;

    attrs.do_not_propagate_mask = 0; // do not hide any events from child window

    attrs.background_pixel = blue.pixel; // background color

    unsigned long attrs_mask = CWEventMask  // enable attrs.event_mask
                             | NoEventMask  // enable attrs.do_not_propagate_mask
                             | CWBackPixel  // enable attrs.background_pixel
                             ;

    // Create and map container window
    new_window_package->window = XCreateWindow(display, RootWindow(display, screen),
                                            0,
                                            0,
                                            WIDTH,
                                            HEIGHT,
                                            1,
                                            CopyFromParent,
                                            InputOutput,
                                            CopyFromParent,
                                            attrs_mask,
                                            &attrs);

    printf("container window is 0x%lX\n",new_window_package->window);
    // Make window visible
    XMapWindow(display, new_window_package->window);

    // Set window title
    XStoreName(display, new_window_package->window, "XLIB container");

    // Get WM_DELETE_WINDOW atom
    new_window_package->wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", True);

    // Subscribe WM_DELETE_WINDOW message
    XSetWMProtocols(display, new_window_package->window, &(new_window_package->wm_delete), 1);
}


int createChildWindow(Window container_window,int argc,char** argv)
{
	// Create child process
	int child_pid = fork();

	if (child_pid == 0) {
		char window_id[64];
		sprintf(window_id, "%lu", container_window);

		char** child_argv = calloc(argc + 1, sizeof(char* ));
		int a;
		for (a = 1; a < argc; ++a) {
			child_argv[a - 1] = argv[a];
		}

		child_argv[argc - 1] = window_id;
		int i;
		for (i=0;i<argc;i++)
		{
			printf("%s ",child_argv[i]);
		}
		printf ("\n");
		execvp(child_argv[0], child_argv);

		fprintf(stderr, "error: can't execute child process!\n");
		exit(1);
	}
	return child_pid;
}


void handleEventLoop(struct windowWithDeletePackage* target_window_package,Display* display,int screen,int child_pid)
{
	// Child window ID and its display
	Display* child_display = NULL;
	Window child_window = 0;

	// Container window event loop
	for (;;) {
		XEvent event;
		XNextEvent(display, &event);

		printf("container_event: %s\n", event_names[event.type]);

		// Map child window when it requests and store its display and window id
		if (event.type == MapRequest) {
			XMapWindow(event.xmaprequest.display, event.xmaprequest.window);

			child_display = event.xmaprequest.display;
			child_window = event.xmaprequest.window;
		}

		// Propagate resize event to child window, and also resize it after MapRequest
		if (event.type == ConfigureNotify || event.type == MapRequest) {
			if (child_window) {
				// Get container window attributes
				XWindowAttributes attrs;
				XGetWindowAttributes(display, target_window_package->window, &attrs);

				// Move and resize child
				XMoveResizeWindow(child_display,
						child_window,
						2, 2, attrs.width - 6, attrs.height - 6);
			}
		}

		// Refresh container window
		if (event.type == Expose) {
			XClearWindow(display, target_window_package->window);
		}

		// Exit if child window was destroyed
		if (event.type == DestroyNotify) {
			fprintf(stderr, "child window destroyed, exiting\n");
			break;
		}

		// Close button
		if (event.type == ClientMessage) {
			if (event.xclient.data.l[0] == target_window_package->wm_delete) {
				break;
			}
		}
	}

	// Kill child process
	kill(child_pid, SIGTERM);
	wait(0);
}

#include "X11/extensions/dri2.h"

int main(int argc, char** argv) 
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s child_program child_arguments..\n", argv[0]);
		exit(1);
	}

	Display* display = XOpenDisplay(NULL);
	if (display == NULL) {
		printf("OpenDisplay for Current Dissplay Failed\n");
		display = XOpenDisplay(":0");
		if (display == NULL) {
			printf("OpenDisplay for :0 Failed\n");
			return 1;
		}
		else
		{
			printf("OpenDisplay for :0 success\n");
		}
	}
	else
	{
		printf("OpenDisplay for Current Dissplay success\n");
	}


	int screen = DefaultScreen(display);

	//struct windowWithDeletePackage container_window;
	//createContainerWindowWithVMDelete(&container_window,display,screen);

	char *device;
	char *driver;


	if (!DRI2Connect(display, container_window.window, 0, &driver, &device)) {
                printf("DRI2Connect failed\n");
                return -1;
        }
	printf("Driver %s\n",driver);
	printf("device %s\n",device);


	int child_pid=createChildWindow(container_window.window,argc,argv);

	handleEventLoop(&container_window,display,screen,child_pid);

	XCloseDisplay(display);
	return 0;
}
