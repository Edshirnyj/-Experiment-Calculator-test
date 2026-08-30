#include "input_handler.h"
#include <string.h>

static InputHandler* global_handler = NULL;
static Renderer* global_renderer = NULL;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void) window;
    (void) scancode;
    (void) mods;
    
    if (!global_handler || key < 0 || key > GLFW_KEY_LAST) { return; }

    if (action == GLFW_PRESS){
        global_handler->key_pressed[key] = true;
        global_handler->key_handled[key] = false;
    } else if (action == GLFW_RELEASE) {
        global_handler->key_pressed[key] = false;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void) window;
    (void) mods;

    if(!global_renderer) { return; }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            renderer_set_mouse_pressed(global_renderer, true);
            Calc* calc = (Calc*)glfwGetWindowUserPointer(window);
            if(calc) {
                renderer_handle_click(global_renderer, calc);
            }
        } else if (action == GLFW_RELEASE) {
            renderer_set_mouse_pressed(global_renderer, false);
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;
    if (global_renderer) {
        renderer_update_mouse(global_renderer, xpos, ypos);
    }
}


void input_handler_init(InputHandler* handler) {
    if (!handler) { return; }
    memset(handler->key_pressed, 0, sizeof(handler->key_pressed));
    memset(handler->key_handled, 0, sizeof(handler->key_handled));
}

void input_handler_process_keyboard(InputHandler* handler, Calc* calc) {
    if (!handler || !calc) { return; }

    // Digits
    for (int i = 0; i <= 9; i++) {
        int key = GLFW_KEY_0 + i;
        if (handler->key_pressed[key] && !handler->key_handled[key]) {
            calc_input_digit(calc, i);
            handler->key_handled[key] = true;
        }
    }

    // Numpad digits
    for (int i = 0; i <= 9; i++) {
        int key = GLFW_KEY_KP_0 + i;
        if (handler->key_pressed[key] && !handler->key_handled[key]) {
            calc_input_digit(calc, i);
            handler->key_handled[key] = true;
        }
    }

    // Operations
    struct { int key; Operation op; } operation_keys[] = {
        {GLFW_KEY_KP_ADD, OPERATION_ADD},
        {GLFW_KEY_KP_SUBTRACT, OPERATION_SUBTRACT},
        {GLFW_KEY_KP_MULTIPLY, OPERATION_MULTIPLY},
        {GLFW_KEY_KP_DIVIDE, OPERATION_DIVIDE},
    };

    for (size_t i = 0; i < sizeof(operation_keys) / sizeof(operation_keys[0]); i++) {
        if (handler->key_pressed[operation_keys[i].key] && !handler->key_handled[operation_keys[i].key]) {
            calc_input_operation(calc, operation_keys[i].op);
            handler->key_handled[operation_keys[i].key] = true;
        }
    }

    // Special keys
    if (handler->key_pressed[GLFW_KEY_ENTER] && !handler->key_handled[GLFW_KEY_ENTER]) {
        calc_calculate(calc);
        handler->key_handled[GLFW_KEY_ENTER] = true;
    }
    if (handler->key_pressed[GLFW_KEY_KP_ENTER] && !handler->key_handled[GLFW_KEY_KP_ENTER]) {
        calc_calculate(calc);
        handler->key_handled[GLFW_KEY_KP_ENTER] = true;
    }
    if (handler->key_pressed[GLFW_KEY_ESCAPE] && !handler->key_handled[GLFW_KEY_ESCAPE]) {
        calc_clear(calc);
        handler->key_handled[GLFW_KEY_ESCAPE] = true;
    }
    if (handler->key_pressed[GLFW_KEY_BACKSPACE] && !handler->key_handled[GLFW_KEY_BACKSPACE]) {
        calc_backspace(calc);
        handler->key_handled[GLFW_KEY_BACKSPACE] = true;
    }
    if(handler->key_pressed[GLFW_KEY_KP_DECIMAL] && !handler->key_handled[GLFW_KEY_KP_DECIMAL]) {
        calc_input_decimal(calc);
        handler->key_handled[GLFW_KEY_KP_DECIMAL] = true;
    }
    if(handler->key_pressed[GLFW_KEY_PERIOD] && !handler->key_handled[GLFW_KEY_PERIOD]) {
        calc_input_decimal(calc);
        handler->key_handled[GLFW_KEY_PERIOD] = true;
    }
}

void input_handler_process_mouse(InputHandler* handler, Renderer* renderer, Calc* calc) {
    (void) handler; // Unused parameter
    (void) renderer; // Unused parameter
    (void) calc; // Unused parameter
    // Mouse input is handled directly in the mouse button callback, so no processing is needed here.
}

void input_handler_setup_callbacks(InputHandler* handler, GLFWwindow* window, Renderer* renderer) {
    if (!handler || !window || !renderer) { return; }

    global_handler = handler;
    global_renderer = renderer;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
}