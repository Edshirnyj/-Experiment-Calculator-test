#define STB_TRUETYPE_IMPLEMENTATION
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
    renderer->cdata = NULL;

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

    // Load font
    const char *font_paths[] = {
        "assets/fonts/RobotoMono-VariableFont_wght.ttf",
        "../assets/fonts/RobotoMono-VariableFont_wght.ttf",
        "./assets/fonts/RobotoMono-VariableFont_wght.ttf",
        "/usr/share/fonts/TTF/RobotoMono-VariableFont_wght.ttf",
        "/usr/share/fonts/truetype/roboto/RobotoMono-VariableFont_wght.ttf"};
    FILE *font_file = NULL;
    const char *used_path = NULL;

    for (int i = 0; i < sizeof(font_paths) / sizeof(font_paths[0]); i++)
    {
        font_file = fopen(font_paths[i], "rb");
        if (font_file)
        {
            used_path = font_paths[i];
            break;
        }
    }

    if (!font_file)
    {
        fprintf(stderr, "Failed to open font file. Tried multiple paths.\n");
        return false;
    }

    printf("Font loaded from: %s\n", used_path);

    fseek(font_file, 0, SEEK_END);
    long font_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    unsigned char *font_buffer = (unsigned char *)malloc(font_size);
    if (!font_buffer)
    {
        fprintf(stderr, "Failed to allocate font buffer\n");
        fclose(font_file);
        return false;
    }
    fread(font_buffer, 1, font_size, font_file);
    fclose(font_file);

    // init stb_truetype
    stbtt_fontinfo info;
    int font_offset = stbtt_GetFontOffsetForIndex(font_buffer, 0);
    if (font_offset < 0 || !stbtt_InitFont(&info, font_buffer, font_offset))
    {
        fprintf(stderr, "Failed to initialize font\n");
        free(font_buffer);
        return false;
    }

    // Create texture for glyphs
    renderer->font_tex_size = 512;
    renderer->cdata = (stbtt_packedchar *)malloc(sizeof(stbtt_packedchar) * 96);
    if (!renderer->cdata)
    {
        fprintf(stderr, "Failed to allocate cdata\n");
        free(font_buffer);
        return false;
    }

    unsigned char *bitmap = (unsigned char *)calloc(renderer->font_tex_size, renderer->font_tex_size);
    if (!bitmap)
    {
        fprintf(stderr, "Failed to allocate bitmap\n");
        free(renderer->cdata);
        free(font_buffer);
        return false;
    }

    stbtt_pack_context pc;
    if (!stbtt_PackBegin(&pc, bitmap, renderer->font_tex_size, renderer->font_tex_size, 0, 1, NULL))
    {
        fprintf(stderr, "Failed to start packing\n");
        return false;
    }

    if (!stbtt_PackFontRange(&pc, font_buffer, 0, 32.0f, 32, 126, renderer->cdata))
    {
        fprintf(stderr, "Failed to pack font range\n");
        stbtt_PackEnd(&pc);
        return false;
    }

    stbtt_PackEnd(&pc);

    glGenTextures(1, &renderer->text_texture);
    glBindTexture(GL_TEXTURE_2D, renderer->text_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, renderer->font_tex_size, renderer->font_tex_size, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL error while creating texture: %d\n", err);
        free(bitmap);
        free(renderer->cdata);
        free(font_buffer);
        return false;
    }

    free(bitmap);
    free(font_buffer);

    const char *labels1[] = {"C", "+/-", "%", "/"};
    for (int i = 0; i < 4; i++)
    {
        buttons[button_count].label = labels1[i];
        buttons[button_count].type = 1;
        buttons[button_count].value = (i == 0) ? -2 : (i == 1) ? -3 : -5;
        button_count++;
    }

    const char *labels2[] = {"7", "8", "9", "*"};
    for (int i = 0; i < 4; i++)
    {
        buttons[button_count].label = labels2[i];
        buttons[button_count].type = 0;
        buttons[button_count].value = (i < 3) ? (7 + i) : OPERATION_MULTIPLY;
        button_count++;
    }

    const char *labels3[] = {"4", "5", "6", "-"};
    for (int i = 0; i < 4; i++)
    {
        buttons[button_count].label = labels3[i];
        buttons[button_count].type = 0;
        buttons[button_count].value = (i < 3) ? (4 + i) : OPERATION_SUBTRACT;
        button_count++;
    }

    const char *labels4[] = {"1", "2", "3", "+"};
    for (int i = 0; i < 4; i++)
    {
        buttons[button_count].label = labels4[i];
        buttons[button_count].type = 0;
        buttons[button_count].value = (i < 3) ? (1 + i) : OPERATION_ADD;
        button_count++;
    }

    const char *labels5[] = {"0", ".", "="};
    buttons[button_count].label = labels5[0];
    buttons[button_count].type = 0;
    buttons[button_count].value = 0;
    button_count++;
    buttons[button_count].label = labels5[1];
    buttons[button_count].type = 1;
    buttons[button_count].value = -6;
    button_count++;
    buttons[button_count].label = labels5[2];
    buttons[button_count].type = 1;
    buttons[button_count].value = -1;
    button_count++;

    buttons[button_count].label = "Backspace";
    buttons[button_count].type = 1;
    buttons[button_count].value = -4;
    button_count++;

    float button_width = (float)(width - BUTTON_PADDING * (BUTTON_COLUMNS + 1)) / BUTTON_COLUMNS;
    float button_height = (float)(height - DISPLAY_HEIGHT - BUTTON_PADDING * (BUTTON_ROWS + 1)) / BUTTON_ROWS;

    for (int i = 0; i < button_count; i++)
    {
        int row = i / BUTTON_COLUMNS;
        int col = i % BUTTON_COLUMNS;
        buttons[i].x = BUTTON_PADDING + col * (button_width + BUTTON_PADDING);
        buttons[i].y = BUTTON_PADDING + row * (button_height + BUTTON_PADDING);
        buttons[i].width = button_width;
        buttons[i].height = button_height;
    }

    fprintf(stderr, "Font loaded successfully!\n");
    return true;
}

void renderer_shutdown(Renderer *renderer)
{
    if (renderer)
    {
        if (renderer->text_texture)
        {
            glDeleteTextures(1, &renderer->text_texture);
        }
        if (renderer->cdata)
        {
            free(renderer->cdata);
            renderer->cdata = NULL;
        }
    }
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
    glColor3f(renderer->display_color[0], renderer->display_color[1], renderer->display_color[2]);
    glBegin(GL_QUADS);
    glVertex2f(0, (float)renderer->height - (float)DISPLAY_HEIGHT);
    glVertex2f((float)renderer->width, (float)renderer->height - (float)DISPLAY_HEIGHT);
    glVertex2f((float)renderer->width, (float)renderer->height);
    glVertex2f(0, (float)renderer->height);
    glEnd();

    const char *expression = calc_get_expression(calc);
    if (expression && strlen(expression) > 0)
    {
        renderer_draw_text(renderer, expression, 15.0f, (float)renderer->height - 25.0f, 1.5f);
    }

    char result[64];
    snprintf(result, sizeof(result), "%.6g", calc_get_current_value(calc));
    renderer_draw_text(renderer, result, 15.0f, (float)renderer->height - 60.0f, 2.0f);

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
                    break; // =
                case -2:
                    calc_clear(calc);
                    break; // C
                case -3:
                    calc_toggle_sign(calc);
                    break; // +/-
                case -4:
                    calc_backspace(calc);
                    break; // Backspace
                case -5:   /* Процент (calc_percent) */
                    break; // % (нужно добавить функцию в calc.c)
                case -6:
                    calc_input_decimal(calc);
                    break; // .
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

float renderer_get_text_width(const char *text, float scale)
{
    if (!text)
        return 0.0f;

    float width = 0.0f;
    for (const char *c = text; *c != '\0'; c++)
    {
        if (*c < 32 || *c > 126)
        {
            width += 10.0f * scale;
            continue;
        }

        stbtt_aligned_quad q;
        float x = 0.0f, y = 0.0f;
        stbtt_GetPackedQuad(NULL, 0, 0, *c - 32, &x, &y, &q, 1);
        width += (q.x1 - q.x0) * (2.0f * scale);
    }

    return width;
}

void renderer_draw_text(Renderer *renderer, const char *text, float x, float y, float scale)
{
    if (!renderer || !text || !renderer->text_texture || !renderer->cdata)
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, renderer->text_texture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(renderer->text_color[0], renderer->text_color[1], renderer->text_color[2], renderer->text_color[3]);

    float start_x = x;
    float current_y = y;
    for (const char *c = text; *c != '\0'; c++)
    {
        if (*c < 32 || *c > 126)
        {
            start_x += 10.0f * scale;
            continue;
        }

        stbtt_aligned_quad q;
        stbtt_GetPackedQuad(renderer->cdata, renderer->font_tex_size, renderer->font_tex_size, *c - 32, &start_x, &current_y, &q, 1);

        float bottom_y = renderer->height - q.y1;
        float top_y = renderer->height - q.y0;
        float left = q.x0;
        float right = q.x1;

        glBegin(GL_QUADS);
        glTexCoord2f(q.s0, q.t1);
        glVertex2f(left, bottom_y);
        glTexCoord2f(q.s1, q.t1);
        glVertex2f(right, bottom_y);
        glTexCoord2f(q.s1, q.t0);
        glVertex2f(right, top_y);
        glTexCoord2f(q.s0, q.t0);
        glVertex2f(left, top_y);
        glEnd();
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void renderer_draw_button_label(Renderer *renderer, const Button *button)
{
    if (!renderer || !button || !button->label)
        return;

    float scale = 1.0f;
    float text_width = renderer_get_text_width(button->label, scale);
    float text_height = 20.0f;

    float x = button->x + (button->width - text_width) / 2.0f;
    float y = button->y + (button->height - text_height) / 2.0f;

    renderer_draw_text(renderer, button->label, x, y, scale);
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