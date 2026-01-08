#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>

// `FORMAT` refers to the type length of the data in bits
#define FORMAT 8

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720
#define BACKGROUND_COLOR 0x1e1e2e

#define CSTR_FMT(str) (sizeof(str) - 1), (str)

int main() {
  int screen_number;
  xcb_connection_t *connection = xcb_connect(NULL, &screen_number);
  assert(!xcb_connection_has_error(connection));

  xcb_intern_atom_cookie_t intern_atom_cookie =
      xcb_intern_atom(connection, true, CSTR_FMT("WM_PROTOCOLS"));
  xcb_intern_atom_reply_t *intern_atom_reply =
      xcb_intern_atom_reply(connection, intern_atom_cookie, NULL);
  xcb_atom_t wm_protocols_prop = intern_atom_reply->atom;
  free(intern_atom_reply);

  intern_atom_cookie =
      xcb_intern_atom(connection, true, CSTR_FMT("WM_DELETE_WINDOW"));
  intern_atom_reply =
      xcb_intern_atom_reply(connection, intern_atom_cookie, NULL);
  xcb_atom_t wm_delete_win_protocol = intern_atom_reply->atom;
  free(intern_atom_reply);

  xcb_screen_t *screen = xcb_aux_get_screen(connection, screen_number);
  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window_aux(
      connection, screen->root_depth, window, screen->root, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
      XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
      &(xcb_create_window_value_list_t){.background_pixel = BACKGROUND_COLOR,
                                        .event_mask = XCB_EVENT_MASK_EXPOSURE});
  xcb_icccm_set_wm_name(connection, window, XCB_ATOM_STRING, FORMAT,
                        CSTR_FMT("Test X11"));
  xcb_icccm_set_wm_protocols(connection, window, wm_protocols_prop, 1,
                             &(xcb_atom_t){wm_delete_win_protocol});

  xcb_map_window(connection, window);
  assert(xcb_flush(connection));

  bool is_running = true;
  while (is_running) {
    xcb_generic_event_t *generic_event;
    while ((generic_event = xcb_poll_for_event(connection))) {
      assert(generic_event);
      switch (XCB_EVENT_RESPONSE_TYPE(generic_event)) {
      // This event is sent by another X11 client window (like a window
      // manager).
      case XCB_CLIENT_MESSAGE: {
        xcb_client_message_event_t *event =
            (xcb_client_message_event_t *)generic_event;
        xcb_client_message_data_t data = event->data;
        if (event->window == window &&
            data.data32[0] == wm_delete_win_protocol) {
          uint32_t timestamp_ms = data.data32[1];
          printf("[DEBUG] Timestamp of when the `WM_DELETE_WINDOW` message was "
                 "sent: "
                 "%i ms\n",
                 timestamp_ms);
          is_running = false;
        }
      } break;

      // This event is sent when the parts of the window needs to redrawn.
      case XCB_EXPOSE: {
        xcb_expose_event_t *event = (xcb_expose_event_t *)generic_event;
        if (event->window == window) {
          printf("[DEBUG] Data about part of the window that needs to be "
                 "redrawn, x: %i, "
                 "y:%i, width: %i, height: %i\n",
                 event->x, event->y, event->width, event->height);
        }
      } break;
      }
    }
    free(generic_event);
  }

  xcb_disconnect(connection);
}
