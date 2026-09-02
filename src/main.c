#include "GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>

#include "calc.h"
#include "input_handler.h"
#include "renderer.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "Calculator"

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(void) {
        // Set error callback
    glfwSetErrorCallback(error_callback);

    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize renderer
    Renderer renderer = {0};
    renderer.window = window;

    if(!renderer_init(&renderer, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)) {
        fprintf(stderr, "Failed to initialize renderer\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;        
    }

    Calc calc;
    calc_init(&calc);

    InputHandler input_handler;
    input_handler_init(&input_handler);

    glfwSetWindowUserPointer(window, &calc);

    input_handler_setup_callbacks(&input_handler, window, &renderer);

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));

    while (!renderer_should_close(&renderer)){
        input_handler_process_keyboard(&input_handler, &calc);

        renderer_begin_frame(&renderer);
        renderer_draw_calc(&renderer, &calc);
        renderer_end_frame(&renderer);
    }

    renderer_shutdown(&renderer);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}