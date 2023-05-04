#include <stdio.h>
#include <math.h>

#include "base/base.h"
#include "gfx/gfx.h"

#if defined(PLATFORM_WINDOWS)
#    define UNICODE
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    include <GL/gl.h>
#elif defined(PLATFORM_LINUX)
#    include <GL/gl.h>
#    include <unistd.h>
#endif

#include "gfx/opengl/opengl.h"
#include "gfx/opengl/opengl_helpers.h"

#include "fpng/fpng.h"

#define WIN_SCALE 1
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080

typedef struct {
    f64 r, i;
} complex;

complex cx_add(complex a, complex b) {
    return (complex){
        .r = a.r + b.r,
        .i = a.i + b.i
    };
}
complex cx_mul(complex a, complex b) {
    return (complex) {
        .r = a.r * b.r - a.i * b.i,
        .i = a.r * b.i + a.i * b.r
    };
}

typedef struct {
    u8 r, g, b, a;
} pixel8;

void render_mandelbrot(pixel8* out, u32 img_width, u32 img_height, complex complex_dim, complex complex_center, u32 iterations) {
    for (u32 y = 0; y < img_height; y++) {
        for (u32 x = 0; x < img_width; x++) {
            complex z = { 0 };
            complex c = {
                (((f64)x / (f64)img_width) - 0.5) * complex_dim.r + complex_center.r,
                (((f64)y / (f64)img_height) - 0.5) * complex_dim.i + complex_center.i
            };

            f32 n = (f32)iterations - 1.0;

            for (u32 i = 0; i < iterations; i++) {
                z = cx_add(cx_mul(z, z), c);

                if (z.r * z.r + z.i * z.i > 4.0) {
                    n = (f32)i;
                    break;
                }
            }

            u32 j = x + y * img_width;
            if (n == (f32)iterations - 1.0) {
                out[j] = (pixel8){ 0, 0, 0, 255 };
            } else {
                out[j] = (pixel8){
                    .r = (u8)((sinf(0.1 * n) * 0.5f + 0.5f) * 255.0f),
                    .g = (u8)((sinf(0.1 * n + 4.188) * 0.5f + 0.5f) * 255.0f),
                    .b = (u8)((sinf(0.1 * n + 2.904) * 0.5f + 0.5f) * 255.0f),
                    .a = 255
                };
                //u32 col = (u32)((n / iterations) * 254.0) + 1;
                //out[j] = (pixel8){ col, col, col, 255 };
            }
        }
    }
}

void mga_err(mga_error err) {
    printf("MGA ERROR %d: %s", err.code, err.msg);
}

int main(void) {
    mga_desc desc = {
        .desired_max_size = MGA_MiB(16),
        .desired_block_size = MGA_KiB(256),
        .error_callback = mga_err
    };
    mg_arena* perm_arena = mga_create(&desc);

    fpng_init();

    gfx_window* win = gfx_win_create(perm_arena, WIDTH, HEIGHT, STR8("Fractal Renderer"));

    pixel8* screen = MGA_PUSH_ZERO_ARRAY(perm_arena, pixel8, IMG_WIDTH * IMG_HEIGHT);
    for (u32 i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
        screen[i].a = 255;
    }

    const char* vert_source = ""
        "#version 330 core\n"
        "layout (location = 0) in vec2 a_pos;"
        "layout (location = 1) in vec2 a_uv;"
        "out vec2 uv;"
        "void main() {"
        "   uv = a_uv;"
        "   gl_Position = vec4(a_pos, 0, 1);"
        "}";
    const char* frag_source = ""
        "#version 330 core\n"
        "#define PI 3.14159265359\n"
        "layout (location = 0) out vec4 out_col;"
        "uniform sampler2D u_texture;"
        "in vec2 uv;"
        "void main() {"
        "    vec4 sample = texture(u_texture, uv);"
        "    out_col = sample;"
        /*"    if (sample.xyz == vec3(0)) {"
        "        out_col = vec4(0, 0, 0, 1);"
        "    } else {"
        "        out_col.r = sin(sample.r * PI) * 0.5 + 0.5;"
        "        out_col.g = sin(sample.r * PI) * 0.5 + 0.5;"
        "        out_col.b = sin(sample.r * PI) * 0.5 + 0.5;"
        "        out_col.a = 1.0;"
        "    }"*/
        "}";

    f32 verts[4 * 6] = {
        -1.0f, -1.0f,   0.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 0.0f,

        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
    };

    u32 vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    u32 vertex_buffer = glh_create_buffer(
        GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW
    );
    u32 shader = glh_create_shader(vert_source, frag_source);

    glEnable(GL_TEXTURE_2D);

    u32 texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT);

    glClearColor(0.45f, 0.65f, 0.77f, 1.0f);


    /*fpng_img img = {
        .channels = 4,
        .width = IMG_WIDTH,
        .height = IMG_HEIGHT,
        .data = (u8*)screen
    };
    string8 out = { 0 };
    fpng_encode_image_to_memory(perm_arena, &img, &out, 0);

    FILE* f = fopen("out.png", "wb");
    fwrite(out.str, 1, out.size, f);
    fclose(f);*/

    complex complex_dim = { 4.0, 4.0 * 9.0 / 16.0 };
    complex complex_center = { 0, 0 };
    vec2 init_pos = { 0 };

    render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, complex_dim, complex_center, 32);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);

    while (!win->should_close) {
        gfx_win_process_events(win);

        if (win->mouse_buttons[0] && !win->prev_mouse_buttons[0]) {
            init_pos = win->mouse_pos;
        }

        if (!win->mouse_buttons[0] && win->prev_mouse_buttons[0]) {
            printf("dim: %f %f, center: %f %f\n", complex_dim.r, complex_dim.i, complex_center.r, complex_center.i);
            
            // -0.5 -> 0.5
            complex norm_center = {
                (((init_pos.x + win->mouse_pos.x) * 0.5) / win->width) - 0.5,
                (((init_pos.y + win->mouse_pos.y) * 0.5) / win->height) - 0.5
            };
            complex_center.r += norm_center.r * complex_dim.r;
            complex_center.i += norm_center.i * complex_dim.i;

            
            complex_dim.r *= -((init_pos.x - win->mouse_pos.x) / win->width);
            complex_dim.i = complex_dim.r * (9.0f / 16.0f);//-((init_pos.y - win->mouse_pos.y) / win->height);
            
            printf("dim: %f %f, center: %f %f\n\n", complex_dim.r, complex_dim.i, complex_center.r, complex_center.i);

            render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, complex_dim, complex_center, 256);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);
        }

        if (win->mouse_buttons[2] && !win->prev_mouse_buttons[2]) {
            printf("saving images\n");

            mga_temp temp = mga_temp_begin(perm_arena);
            
            u32 i = 0;
            b32 done = false;
            while (!done) {
                if (complex_dim.r >= 4.0f)
                    done = true;
                
                render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, complex_dim, complex_center, 512);
                complex_dim.r *= 2.0;
                complex_dim.i *= 2.0;
                
                fpng_img img = {
                    .channels = 4,
                    .width = IMG_WIDTH,
                    .height = IMG_HEIGHT,
                    .data = (u8*)screen
                };
                string8 out = { 0 };
                fpng_encode_image_to_memory(perm_arena, &img, &out, 0);

                char file_path[256] = { 0 };
                sprintf(file_path, "out/img_%.4u.png", i);
                i++;
                
                FILE* f = fopen(file_path, "wb");
                fwrite(out.str, 1, out.size, f);
                fclose(f);

                mga_temp_end(temp);
                
                printf("image %d, dim: %f %f\n", i - 1, complex_dim.r, complex_dim.i);
            }
        }

        gfx_win_clear(win);

        glUseProgram(shader);
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(sizeof(f32) * 2));

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        gfx_win_swap_buffers(win);

        #if defined(PLATFORM_LINUX)
        usleep(16000);
        #endif
    }

    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);

    gfx_win_destroy(win);

    mga_destroy(perm_arena);

    return 0;
}
