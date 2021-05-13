// Simple XCB application for opening a window and drawing a box in it

// To compile it using GNU, use:
// gcc x.c -lxcb

#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

//for getpid
#include <sys/types.h>
#include <unistd.h>

void createNewXCBWindow(xcb_window_t *pw, xcb_connection_t *c, xcb_screen_t *s, xcb_gcontext_t *pg)
{
  uint32_t mask;
  uint32_t values[2];
  // get the first screen
  s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;

  // create black graphics context
  *pg = xcb_generate_id(c);
  printf("new g ID is 0x%x\n", *pg);

  *pw = s->root;
  mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = s->black_pixel;
  values[1] = 0;
  xcb_create_gc(c, *pg, *pw, mask, values);

  // create window
  *pw = xcb_generate_id(c);
  printf("new w ID is 0x%x\n", *pw);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = s->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
  xcb_create_window(c, s->root_depth, *pw, s->root,
                    10, 10, 100, 100, 1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual,
                    mask, values);

  // map (show) the window
  xcb_map_window(c, *pw);

  xcb_flush(c);
}

void loopForEvent(xcb_connection_t *c, xcb_window_t w, xcb_gcontext_t g)
{
  int done = 0;
  xcb_generic_event_t *e;
  xcb_rectangle_t r = {20, 20, 60, 60};

  char buf[1024];
  sprintf(buf, "lsof -p %d", getpid());
  system(buf);

  while (!done && (e = xcb_wait_for_event(c)))
  {
    switch (e->response_type & ~0x80)
    {
    case XCB_EXPOSE: // draw or redraw the window
      xcb_poly_fill_rectangle(c, w, g, 1, &r);
      xcb_flush(c);
      printf("EXPOSE event\n");
      break;
    case XCB_KEY_PRESS: // exit on key press
      done = 1;
      printf("KEY PRESS event\n");
      break;
    }
    free(e);
  }
}
int main(void)
{
  xcb_connection_t *c;
  xcb_screen_t *s;
  xcb_window_t w;
  xcb_gcontext_t g;

  // open connection to the server
  c = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(c))
  {
    printf("Cannot open display for Current Display \n");
    c = xcb_connect(":0", NULL);
    if (xcb_connection_has_error(c))
    {
      printf("Cannot open display for :0 Display \n");
      exit(EXIT_FAILURE);
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

  createNewXCBWindow(&w, c, s, &g);

  // event loop
  loopForEvent(c, w, g);

  // close connection to server
  xcb_disconnect(c);

  exit(EXIT_SUCCESS);
}