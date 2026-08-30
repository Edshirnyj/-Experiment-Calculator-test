#include "GLFW/glfw3.h"
#include "calc.h"
#include "renderer.h"

typedef struct InputHandler_struct
{
    bool key_pressed[GLFW_KEY_LAST + 1];
    bool key_handled[GLFW_KEY_LAST + 1];
} InputHandler;

void input_handler_init(InputHandler *handler);
void input_handler_process_keyboard(InputHandler *handler, Calc *calc);
void input_handler_process_mouse(InputHandler *handler, Renderer *renderer, Calc *calc);
void input_handler_setup_callbacks(InputHandler *handler, GLFWwindow *window, Renderer *renderer);