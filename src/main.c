#include <stdio.h>

#include "base/base.h"
#include "gfx/gfx.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/gl.h>
#include "gfx/opengl/opengl.h"

#include "fpng/fpng.h"

#define WIDTH 1280
#define HEIGHT 720

void mga_err(mga_error err) {
    printf("MGA ERROR %d: %s", err.code, err.msg);
}

static mg_arena* perm_arena = NULL;

int main(void) {
    mga_desc desc = {
        .desired_max_size = MGA_MiB(8),
        .desired_block_size = MGA_KiB(256),
        .error_callback = mga_err
    };
    perm_arena = mga_create(&desc);

    fpng_init();

    gfx_window* win = gfx_win_create(perm_arena, 640, 360, STR8("Fractal Renderer"));
    gfx_win_make_current(win);

    glClearColor(139.0f/255.0f, 168.0f/255.0f, 196.0f/255.0f, 1.0f);

    while (!win->should_close) {
        gfx_win_process_events(win);

        gfx_win_clear(win);

        gfx_win_swap_buffers(win);
    }

    gfx_win_destroy(win);

    mga_destroy(perm_arena);

    return 0;
}
