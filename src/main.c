#include <stdio.h>

#include "base/base.h"
#include "gfx/gfx.h"

#if defined(PLATFORM_WINDOWS)
#    define UNICODE
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    include <GL/gl.h>
#elif defined(PLATFORM_LINUX)
#    include <GL/gl.h>
#endif

#include "gfx/opengl/opengl.h"
#include "gfx/opengl/opengl_helpers.h"

#include "fpng/fpng.h"

#define WIN_SCALE 1
#define WIDTH (u32)(320 * WIN_SCALE)
#define HEIGHT (u32)(180 * WIN_SCALE)

typedef struct {
    f32 r, i;
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

void mga_err(mga_error err) {
    printf("MGA ERROR %d: %s", err.code, err.msg);
}

int main(void) {
    mga_desc desc = {
        .desired_max_size = MGA_MiB(8),
        .desired_block_size = MGA_KiB(256),
        .error_callback = mga_err
    };
    mg_arena* perm_arena = mga_create(&desc);

    fpng_init();

    gfx_window* win = gfx_win_create(perm_arena, WIDTH, HEIGHT, STR8("Fractal Renderer"));

    pixel8* screen = MGA_PUSH_ZERO_ARRAY(perm_arena, pixel8, WIDTH * HEIGHT);
    for (u32 i = 0; i < WIDTH * HEIGHT; i++) {
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
        "layout (location = 0) out vec4 out_col;"
        "uniform sampler2D u_texture;"
        "in vec2 uv;"
        "void main() {"
        "   out_col = texture(u_texture, uv);"
        "}";

    f32 verts[4 * 6] = {
        -1.0f, -1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,

        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
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

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, WIDTH, HEIGHT);

    glClearColor(0.45f, 0.65f, 0.77f, 1.0f);

    u32 iterations = 128;
    for (u32 x = 0; x < WIDTH; x++) {
        for (u32 y = 0; y < HEIGHT; y++) {
            complex z = { 0 };
            complex c = {
                ((f32)x / (f32)WIDTH) * 4.0f - 2.0f,
                ((f32)y / (f32)HEIGHT) * 4.0f - 2.0f
            };

            f32 n = (f32)iterations - 1.0f;

            for (u32 i = 0; i < iterations; i++) {
                z = cx_add(cx_mul(z, z), c);

                if (z.r * z.r + z.i * z.i > 4.0f) {
                    n = (f32)i;
                    break;
                }
            }

            u32 j = x + y * WIDTH;
            //if (n == (f32)iterations - 1.0f) {
            //    screen[j] = (pixel8){ 0, 0, 0, 1 };
            //} else {
                u32 col = (u32)((n / iterations) * 255.0f);
                screen[j] = (pixel8){ col, col, col, 1 };
            //}
        }
    }

    while (!win->should_close) {
        gfx_win_process_events(win);

        gfx_win_clear(win);

        glUseProgram(shader);
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(sizeof(f32) * 2));

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        gfx_win_swap_buffers(win);
    }

    glDeleteProgram(shader);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);

    gfx_win_destroy(win);

    mga_destroy(perm_arena);

    return 0;
}
