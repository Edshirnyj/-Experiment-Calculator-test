#include "renderer.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAX_BUTTONS 24
#define DISPLAY_HEIGHT 100
#define BUTTON_PADDING 10
#define BUTTON_ROWS 6
#define BUTTON_COLUMNS 4

static Button buttons[MAX_BUTTONS];
static int button_count = 0;

bool renderer_init(Renderer *renderer, int width, int height, const char *title)
{
    if (!renderer || !title)
        return false;

    renderer->width = width;
    renderer->height = height;
    renderer->mouse_x = 0;
    renderer->mouse_y = 0;
    renderer->mouse_pressed = false;
    renderer->text_texture = 0;

    renderer->button_color[0] = 0.3f;
    renderer->button_color[1] = 0.3f;
    renderer->button_color[2] = 0.3f;
    renderer->button_color[3] = 1.0f;

    renderer->button_hover_color[0] = 0.4f;
    renderer->button_hover_color[1] = 0.4f;
    renderer->button_hover_color[2] = 0.4f;
    renderer->button_hover_color[3] = 1.0f;

    renderer->text_color[0] = 1.0f;
    renderer->text_color[1] = 1.0f;
    renderer->text_color[2] = 1.0f;
    renderer->text_color[3] = 1.0f;

    renderer->display_color[0] = 0.1f;
    renderer->display_color[1] = 0.1f;
    renderer->display_color[2] = 0.1f;
    renderer->display_color[3] = 1.0f;

    renderer->background_color[0] = 0.2f;
    renderer->background_color[1] = 0.2f;
    renderer->background_color[2] = 0.2f;
    renderer->background_color[3] = 1.0f;

    button_count = 0;

    const char *digit_labels[] = {"7", "8", "9", "4", "5", "6", "1", "2", "3", "0"};
    for (int i = 0; i < 10; i++)
    {
        buttons[button_count].label = digit_labels[i];
        buttons[button_count].type = 0;
        buttons[button_count].value = i;
        button_count++;
    }

    struct
    {
        const char *label;
        int value;
    } ops[] = {
        {"+", OPERATION_ADD},
        {"-", OPERATION_SUBTRACT},
        {"*", OPERATION_MULTIPLY},
        {"/", OPERATION_DIVIDE},
        {"^", OPERATION_POWER},
        {"sqrt", OPERATION_SQRT},
        {"sin", OPERATION_SIN},
        {"cos", OPERATION_COS},
        {"tan", OPERATION_TAN},
        {"=", -1},
        {"C", -2},
        {"+/-", -3},
        {"Backspace", -4},
    };

    for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
    {
        buttons[button_count].label = ops[i].label;
        buttons[button_count].type = 1;
        buttons[button_count].value = ops[i].value;
        button_count++;
    }

    float button_width = (float)(width - BUTTON_PADDING * (BUTTON_COLUMNS + 1)) / BUTTON_COLUMNS;
    float button_height = (float)(height - DISPLAY_HEIGHT - BUTTON_PADDING * (BUTTON_ROWS + 1)) / BUTTON_ROWS;

    for (int i = 0; i < button_count; i++)
    {
        int row = i / 4;
        int col = i % 4;
        buttons[i].x = BUTTON_PADDING + col * (button_width + BUTTON_PADDING);
        buttons[i].y = height - DISPLAY_HEIGHT - BUTTON_PADDING - row * (button_height + BUTTON_PADDING);
        buttons[i].width = button_width;
        buttons[i].height = button_height;
    }

    return true;
}

void renderer_shutdown(Renderer *renderer)
{
    (void)renderer;
}

void renderer_begin_frame(Renderer *renderer)
{
    if (!renderer || !renderer->window)
    {
        return;
    }

    glfwPollEvents();

    renderer_setup_orthographic(renderer);

    glClearColor(renderer->background_color[0],
                 renderer->background_color[1],
                 renderer->background_color[2],
                 renderer->background_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderer_end_frame(Renderer *renderer)
{
    if (!renderer || !renderer->window)
    {
        return;
    }

    glDisable(GL_BLEND);
    glfwSwapBuffers(renderer->window);
}

void renderer_draw_calc(Renderer *renderer, const Calc *calc)
{
    if (!renderer || !calc)
    {
        return;
    }

    // Draw display
    glColor3f(renderer->display_color[0],
              renderer->display_color[1],
              renderer->display_color[2]);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f((float)renderer->width, 0);
    glVertex2f((float)renderer->width, (float)DISPLAY_HEIGHT);
    glVertex2f(0, (float)DISPLAY_HEIGHT);
    glEnd();

    const char *expression = calc_get_expression(calc);
    if (expression && strlen(expression) > 0)
    {
        renderer_draw_text(renderer, expression, 10.0f, 10.0f, 2.0f);
    }

    char result[64];
    snprintf(result, sizeof(result), "%.6g", calc_get_current_value(calc));
    renderer_draw_text(renderer, result, 10.0f, 50.0f, 3.0f);

    // Draw buttons
    for (int i = 0; i < button_count; i++)
    {
        bool hovered = renderer_is_mouse_pressed(&buttons[i], renderer);

        if (hovered && renderer->mouse_pressed)
        {
            glColor3f(renderer->button_hover_color[0] + 0.8f,
                      renderer->button_hover_color[1] + 0.8f,
                      renderer->button_hover_color[2] + 0.8f);
        }
        else if (hovered)
        {
            glColor3f(renderer->button_hover_color[0],
                      renderer->button_hover_color[1],
                      renderer->button_hover_color[2]);
        }
        else
        {
            if (buttons[i].type == 0)
            {
                glColor3f(renderer->button_color[0] * 0.8f,
                          renderer->button_color[1] * 0.8f,
                          renderer->button_color[2] * 0.8f);
            }
            else if (buttons[i].type == 1)
            {
                glColor3f(0.25f, 0.35f, 0.5f);
            }
            else
            {
                glColor3f(0.4f, 0.25f, 0.25f);
            }
        }

        glBegin(GL_QUADS);
        glVertex2f(buttons[i].x, buttons[i].y);
        glVertex2f(buttons[i].x + buttons[i].width, buttons[i].y);
        glVertex2f(buttons[i].x + buttons[i].width, buttons[i].y + buttons[i].height);
        glVertex2f(buttons[i].x, buttons[i].y + buttons[i].height);
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(buttons[i].x, buttons[i].y);
        glVertex2f(buttons[i].x + buttons[i].width, buttons[i].y);
        glVertex2f(buttons[i].x + buttons[i].width, buttons[i].y + buttons[i].height);
        glVertex2f(buttons[i].x, buttons[i].y + buttons[i].height);
        glEnd();

        renderer_draw_button_label(renderer, &buttons[i]);
    }
}

void renderer_update_mouse(Renderer *renderer, double xpos, double ypos)
{
    if (!renderer)
    {
        return;
    }
    renderer->mouse_x = (int)xpos;
    renderer->mouse_y = renderer->height - (int)ypos;
}

void renderer_set_mouse_pressed(Renderer *renderer, bool pressed)
{
    if (!renderer)
    {
        return;
    }
    renderer->mouse_pressed = pressed;
}

bool renderer_is_mouse_pressed(const Button *button, const Renderer *renderer)
{
    if (!button || !renderer)
    {
        return false;
    }

    return renderer->mouse_x >= button->x &&
           renderer->mouse_x <= button->x + button->width &&
           renderer->mouse_y >= button->y &&
           renderer->mouse_y <= button->y + button->height;
}

void renderer_handle_click(Renderer *renderer, Calc *calc)
{
    if (!renderer || !calc)
    {
        return;
    }

    for (int i = 0; i < button_count; i++)
    {
        if (renderer_is_mouse_pressed(&buttons[i], renderer))
        {
            if (buttons[i].type == 0)
            {
                // Digit
                calc_input_digit(calc, buttons[i].value);
            }
            else if (buttons[i].type == 1)
            {
                // Operation or special
                switch (buttons[i].value)
                {
                case -1:
                    calc_calculate(calc);
                    break;
                case -2:
                    calc_clear(calc);
                    break;
                case -3:
                    calc_toggle_sign(calc);
                    break;
                default:
                    calc_input_operation(calc, (Operation)buttons[i].value);
                    break;
                }
            }

            break;
        }
    }
}

bool renderer_should_close(const Renderer *renderer)
{
    return renderer && renderer->window && glfwWindowShouldClose(renderer->window);
}

void renderer_draw_text(Renderer *renderer, const char *text, float x, float y, float scale)
{
    if (!renderer || !text)
        return;

    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    glColor3f(renderer->text_color[0], renderer->text_color[1], renderer->text_color[2]);

    for (const char *c = text; *c != '\0'; c++)
    {
        glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(8.0f, 0.0f);
        glEnd();

        glTranslatef(10.0f, 0.0f, 0.0f);
    }

    glPopMatrix();
}

void renderer_draw_button_label(Renderer *renderer, const Button *button)
{
    if (!renderer || !button || !button->label)
        return;

    float text_width = strlen(button->label) * 10.0f;
    float text_height = 20.0f;

    float text_x = button->x + (button->width - text_width) / 2.0f;
    float text_y = button->y + (button->height - text_height) / 2.0f;

    glColor3f(renderer->text_color[0], renderer->text_color[1], renderer->text_color[2]);

    glPushMatrix();
    glTranslatef(text_x, text_y, 0);

    for (const char *c = button->label; *c != '\0'; c++)
    {
        glBegin(GL_LINE_STRIP);
        switch (*c)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            glVertex2f(0.0f, 0.0f);
            glVertex2f(8.0f, 0.0f);
            glVertex2f(8.0f, 12.0f);
            glVertex2f(0.0f, 12.0f);
            glVertex2f(0.0f, 0.0f);
            break;
        case '+':
            glVertex2f(4.0f, 0.0f);
            glVertex2f(4.0f, 8.0f);
            glVertex2f(0.0f, 4.0f);
            glVertex2f(8.0f, 4.0f);
            break;
        case '-':
            glVertex2f(0.0f, 4.0f);
            glVertex2f(8.0f, 4.0f);
            break;
        case '*':
            glVertex2f(0.0f, 0.0f);
            glVertex2f(8.0f, 8.0f);
            glVertex2f(8.0f, 0.0f);
            glVertex2f(0.0f, 8.0f);
            break;
        case '/':
            glVertex2f(8.0f, 0.0f);
            glVertex2f(0.0f, 8.0f);
            break;
        case '=':
            glVertex2f(0.0f, 2.0f);
            glVertex2f(8.0f, 8.0f);
            glVertex2f(8.0f, 0.0f);
            glVertex2f(0.0f, 8.0f);
            break;
        case '.':
            glVertex2f(4.0f, 0.0f);
            glVertex2f(4.0f, 8.0f);
            break;
        default:
            glVertex2f(0.0f, 4.0f);
            glVertex2f(8.0f, 4.0f);
            break;
        }

        glEnd();
        glTranslatef(10.0f, 0.0f, 0.0f);
    }

    glPopMatrix();
}

void renderer_setup_orthographic(Renderer *renderer)
{
    if (!renderer)
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)renderer->width, 0.0, (double)renderer->height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}