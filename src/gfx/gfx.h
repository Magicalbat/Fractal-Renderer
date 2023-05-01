#ifndef GFX_H
#define GFX_H

#include "base/base.h"

typedef struct _gfx_win_backend _gfx_win_backend;

typedef struct {
    string8 title;
    u32 width, height;

    b32 should_close;

    _gfx_win_backend* backend;
} gfx_window;

gfx_window* gfx_win_create(mg_arena* arena, u32 width, u32 height, string8 title);
void gfx_win_destroy(gfx_window* win);

void gfx_win_process_events(gfx_window* win);

void gfx_win_make_current(gfx_window* win);
void gfx_win_clear(gfx_window* win);
void gfx_win_swap_buffers(gfx_window* win);

#endif // GFX_H
