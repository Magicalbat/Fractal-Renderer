#include "base/base_defs.h"

#ifdef PLATFORM_LINUX

#include "gfx/gfx.h"
#include "opengl.h"

#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/glx.h>
#include <GL/gl.h>

typedef struct _gfx_win_backend {
    Display* display;
    i32 screen;
    GLXFBConfig fb_config;
    Window window;
    GLXContext gl_context;
    Atom del_atom;
} _gfx_win_backend;

#define X(ret, name, args) gl_##name##_func name = NULL;
#   include "opengl_funcs.h"
#undef X

gfx_window* gfx_win_create(mg_arena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = MGA_PUSH_ZERO_STRUCT(arena, gfx_window);

    *win = (gfx_window){
        .title = title,
        .width = width,
        .height = height,
        .backend = MGA_PUSH_ZERO_STRUCT(arena, _gfx_win_backend)
    };

    win->backend->display = XOpenDisplay(NULL);
    if (win->backend->display == NULL) {
        fprintf(stderr, "Failed to open X11 display");
        return NULL;
    }

    win->backend->screen = DefaultScreen(win->backend->display);
    i32 major_version = 0;
    i32 minor_version = 0;
    glXQueryVersion(win->backend->display, &major_version, &minor_version);
    if (major_version <= 0 || minor_version <= 1) {
        XCloseDisplay(win->backend->display);
        fprintf(stderr, "Invalid GLX version");
        return NULL;
    }

    i32 fbc_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 0,
        GLX_STENCIL_SIZE, 0,
        None
    };

    i32 fb_count = 0;
    GLXFBConfig* fbc = glXChooseFBConfig(win->backend->display, win->backend->screen, fbc_attribs, &fb_count);
    if (fbc == NULL) {
        XCloseDisplay(win->backend->display);
        fprintf(stderr, "Failed to retrieve framebuffer");
        return NULL;
    }

    i32 best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 9999;
    for (i32 i = 0; i < fb_count; i++) {
        XVisualInfo* vi = glXGetVisualFromFBConfig(win->backend->display, fbc[i]);

        if (vi == NULL) {
            XFree(vi);
            continue;
        }

        i32 sample_bufs = 0, samples = 0;
        glXGetFBConfigAttrib(win->backend->display, fbc[i], GLX_SAMPLE_BUFFERS, &sample_bufs);
        glXGetFBConfigAttrib(win->backend->display, fbc[i], GLX_SAMPLES, &samples);

        if (best_fbc < 0 || (sample_bufs && samples > best_num_samp)) {
            best_fbc = i;
            best_num_samp = samples;
        }
        if (worst_fbc < 0 || (!sample_bufs || samples < worst_num_samp)) {
            worst_fbc = i;
            worst_num_samp = samples;
        }

        XFree(vi);
    }

    win->backend->fb_config = fbc[best_fbc];

    XFree(fbc);

    XVisualInfo* visual = glXGetVisualFromFBConfig(win->backend->display, win->backend->fb_config);
    if (visual == NULL) {
        XCloseDisplay(win->backend->display);
        fprintf(stderr, "Cannot create visual window");
        
        return NULL;
    }

    XSetWindowAttributes window_attribs = {
        .border_pixel = BlackPixel(win->backend->display, win->backend->screen),
        .background_pixel = WhitePixel(win->backend->display, win->backend->screen),
        .override_redirect = True,
        .colormap = XCreateColormap(win->backend->display, RootWindow(win->backend->display, win->backend->screen), visual->visual, AllocNone),
        .event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask // | KeyPressMask | KeyReleaseMask
    };

    win->backend->window = XCreateWindow(
        win->backend->display,
        RootWindow(win->backend->display, win->backend->screen),
        0, 0, width, height, 0,
        visual->depth, InputOutput, visual->visual,
        CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
        &window_attribs
    );

    win->backend->del_atom = XInternAtom(win->backend->display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(win->backend->display, win->backend->window, &win->backend->del_atom, 1);

    XMapWindow(win->backend->display, win->backend->window);

    XFree(visual);

    // TODO: createContexAttribsARB

    win->backend->gl_context = glXCreateNewContext(win->backend->display, win->backend->fb_config, GLX_RGBA_TYPE, 0, true);

    XSync(win->backend->display, false);

    XFreeColormap(win->backend->display, window_attribs.colormap);

    glXMakeCurrent(win->backend->display, win->backend->window, win->backend->gl_context);
    
    glViewport(0, 0, width, height);

    #define X(ret, name, args) name = (gl_##name##_func)glXGetProcAddress((const GLubyte*)#name);
    #    include "opengl_funcs.h"
    #undef X

    mga_temp scratch = mga_scratch_get(NULL, 0);
    u8* title_cstr = str8_to_cstr(scratch.arena, title);
    XStoreName(win->backend->display, win->backend->window, (char*)title_cstr);
    mga_scratch_release(scratch);

    return win;
}
void gfx_win_destroy(gfx_window* win) {
    glXDestroyContext(win->backend->display, win->backend->gl_context);

    XDestroyWindow(win->backend->display, win->backend->window);
    XCloseDisplay(win->backend->display);
}

void gfx_win_process_events(gfx_window* win) {
    memcpy(win->prev_mouse_buttons, win->mouse_buttons, sizeof(win->prev_mouse_buttons));
    
    while (XPending(win->backend->display)) {
        XEvent e = { 0 };
        XNextEvent(win->backend->display, &e);

        switch(e.type) {
            case Expose: {
                glViewport(0, 0, e.xexpose.width, e.xexpose.height);
                win->width = e.xexpose.width;
                win->height = e.xexpose.height;
            } break;
            case ButtonPress: {
                win->mouse_buttons[e.xbutton.button - 1] = true;
            } break;
            case ButtonRelease: {
                win->mouse_buttons[e.xbutton.button - 1] = false;
            } break;
            case MotionNotify: {
                win->mouse_pos.x = (f32)e.xmotion.x;
                win->mouse_pos.y = (f32)e.xmotion.y;
            } break;
            case ClientMessage: {
                if ((i64)e.xclient.data.l[0] == (i64)win->backend->del_atom) {
                    win->should_close = true;
                }
            } break;
        }
    }
}

void gfx_win_make_current(gfx_window* win) {
    glXMakeCurrent(win->backend->display, win->backend->window, win->backend->gl_context);
}
void gfx_win_clear(gfx_window* win) {
    UNUSED(win);
    
    glClear(GL_COLOR_BUFFER_BIT);
}
void gfx_win_swap_buffers(gfx_window* win) {
    glXSwapBuffers(win->backend->display, win->backend->window);
}

#endif // PLATFORM_LINUX
