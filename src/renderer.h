#include "GLFW/glfw3.h"
#include "calc.h"

typedef struct Renderer_struct
{
    GLFWwindow *window;
    int width;
    int height;
    float button_color[4];
    float button_hover_color[4];
    float text_color[4];
    float display_color[4];
    float background_color[4];
    int mouse_x;
    int mouse_y;
    bool mouse_pressed;
    GLuint text_texture;
} Renderer;

typedef struct Button_struct
{
    float x, y, width, height;
    const char *label;
    int type;
    int value;
} Button;

bool renderer_init(Renderer *renderer, int width, int height, const char *title);
void renderer_shutdown(Renderer *renderer);
void renderer_begin_frame(Renderer *renderer);
void renderer_end_frame(Renderer *renderer);
void renderer_draw_calc(Renderer *renderer, const Calc *calc);
void renderer_update_mouse(Renderer *renderer, double xpos, double ypos);
void renderer_set_mouse_pressed(Renderer *renderer, bool pressed);
bool renderer_is_mouse_pressed(const Button *button, const Renderer *renderer);
void renderer_handle_click(Renderer *renderer, Calc *calc);
bool renderer_should_close(const Renderer *renderer);

void renderer_draw_text(Renderer *renderer, const char *text, float x, float y, float scale);
void renderer_draw_button_label(Renderer *renderer, const Button *button);
void renderer_setup_orthographic(Renderer *renderer);