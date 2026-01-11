#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include <glad/glx.h>

#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_util.h>

// `FORMAT` refers to the type length of the data in bits
#define FORMAT 8

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define CSTR_FMT(str) (sizeof(str) - 1), (str)

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(205.0f / 255.0f, "
                                   "214.0f / 255.0f, 244.0f / 255.0f, 1.0f);\n"
                                   "}\n\0";

int main() {
  Display *display = XOpenDisplay(0);
  int screen_number = DefaultScreen(display);
  assert(display);
  // NOTE: Be sure to change it right after creating the display.
  XSetEventQueueOwner(display, XCBOwnsEventQueue);

  int glx_version = gladLoaderLoadGLX(display, screen_number);
  assert(glx_version);
  printf("[DEBUG] Loaded GLX %d.%d\n", GLAD_VERSION_MAJOR(glx_version),
         GLAD_VERSION_MINOR(glx_version));

  xcb_connection_t *connection = XGetXCBConnection(display);
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

  XVisualInfo *visual_info = glXChooseVisual(
      display, screen_number, (GLint[]){GLX_RGBA, GLX_DOUBLEBUFFER});

  uint32_t colormap = xcb_generate_id(connection);
  xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, colormap,
                      screen->root, (xcb_visualid_t)(visual_info->visualid));

  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window_aux(
      connection, screen->root_depth, window, screen->root, 0, 0, WIN_WIDTH,
      WIN_HEIGHT, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
      (xcb_visualid_t)visual_info->visualid,
      XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP,
      &(xcb_create_window_value_list_t){.background_pixel = 0x1e1e2e,
                                        .colormap = colormap,
                                        .event_mask = XCB_EVENT_MASK_EXPOSURE});

  xcb_icccm_set_wm_name(connection, window, XCB_ATOM_STRING, FORMAT,
                        CSTR_FMT("Test X11"));
  xcb_icccm_set_wm_protocols(connection, window, wm_protocols_prop, 1,
                             &(xcb_atom_t){wm_delete_win_protocol});

  xcb_map_window(connection, window);
  assert(xcb_flush(connection));

  GLXContext glx_context = glXCreateContext(display, visual_info, NULL, True);
  glXMakeCurrent(display, window, glx_context);

  // Ensure the context is created before trying to load GL functions.
  int gl_version = gladLoaderLoadGL();
  assert(gl_version);
  printf("[DEBUG] Loaded GL %d.%d\n", GLAD_VERSION_MAJOR(gl_version),
         GLAD_VERSION_MINOR(gl_version));

  glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLfloat vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                        0.0f,  0.0f,  0.5f, 0.0f};

  // Create reference containers for the Vartex Array Object and the Vertex
  // Buffer Object
  GLuint VAO, VBO;

  // Generate the VAO and VBO with only 1 object each
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Configure the Vertex Attribute so that OpenGL knows how to read the VBO
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  // Enable the Vertex Attribute so that OpenGL knows to use it
  glEnableVertexAttribArray(0);

  // Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO
  // and VBO we created
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  bool is_running = true;
  while (is_running) {
    glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 46.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glXSwapBuffers(display, window);

    xcb_generic_event_t *generic_event = NULL;
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

      // This event is sent when the parts of the window needs to drawn.
      case XCB_EXPOSE: {
        xcb_expose_event_t *event = (xcb_expose_event_t *)generic_event;
        if (event->window == window) {
          printf("[DEBUG] Data about part of the window that needs to be "
                 "drawn, x: %i, "
                 "y:%i, width: %i, height: %i\n",
                 event->x, event->y, event->width, event->height);
        }
      } break;
      }
      free(generic_event);
    }
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  glXMakeContextCurrent(display, None, None, NULL);
  glXDestroyContext(display, glx_context);

  xcb_free_colormap(connection, colormap);
  xcb_destroy_window(connection, window);

  XCloseDisplay(display);

  gladLoaderUnloadGLX();
}
