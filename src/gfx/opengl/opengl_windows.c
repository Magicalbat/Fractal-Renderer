#include "gfx/gfx.h"

#include "opengl.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/gl.h>

// TODO: error checking

typedef struct _gfx_win_backend {
    HINSTANCE instance;
    WNDCLASS win_class;
    HWND window;
    HDC device_context;
    HGLRC gl_context;
} _gfx_win_backend;

#define X(ret, name, args) gl_##name##_func name = NULL;
#   include "opengl_funcs.h"
#undef X

static HMODULE w32_opengl_module = NULL;

static void* w32_gl_load(const char* name);

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

#define WGL_SWAP_METHOD_ARB                       0x2007
#define WGL_SWAP_EXCHANGE_ARB                     0x2028
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023
#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

typedef BOOL WINAPI (wglChoosePixelFormatARB_func)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC WINAPI (wglCreateContextAttribsARB_func)(HDC hdc, HGLRC hShareContext, const int *attribList);

static LRESULT CALLBACK w32_window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

gfx_window* gfx_win_create(mg_arena* arena, u32 width, u32 height, string8 title) {
    gfx_window* win = MGA_PUSH_ZERO_STRUCT(arena, gfx_window);

    *win = (gfx_window){ 
        .title = title,
        .width = width,
        .height = height,
        .backend = MGA_PUSH_ZERO_STRUCT(arena, _gfx_win_backend)
    };

    win->backend->instance = GetModuleHandle(0);

    if (w32_opengl_module == NULL) {
        w32_opengl_module = LoadLibraryW(L"opengl32.dll");
    }

    wglChoosePixelFormatARB_func* wglChoosePixelFormatARB = NULL;
    wglCreateContextAttribsARB_func* wglCreateContextAttribsARB = NULL;

    // Bootstrap window
    {
        WNDCLASS bootstrap_wnd_class = (WNDCLASSW){
            .hInstance = win->backend->instance,
            .lpfnWndProc = DefWindowProc,
            .lpszClassName = L"Bootstrap Window Class"
        };
        RegisterClassW(&bootstrap_wnd_class);

        HWND bootstrap_window = CreateWindowW(
            bootstrap_wnd_class.lpszClassName,
            NULL, 0,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, win->backend->instance, 0
        );

        HDC bootstrap_dc = GetDC(bootstrap_window);

        PIXELFORMATDESCRIPTOR pfd = (PIXELFORMATDESCRIPTOR){
            .nSize = sizeof(PIXELFORMATDESCRIPTOR),
            .nVersion = 1,
            .dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW,
            .iPixelType = PFD_TYPE_RGBA,
            .cColorBits = 24,
            .iLayerType = PFD_MAIN_PLANE
        };

        i32 pixel_format = ChoosePixelFormat(bootstrap_dc, &pfd);
        SetPixelFormat(bootstrap_dc, pixel_format, &pfd);

        HGLRC bootstrap_context = wglCreateContext(bootstrap_dc);
        wglMakeCurrent(bootstrap_dc, bootstrap_context);

        wglChoosePixelFormatARB = (wglChoosePixelFormatARB_func*)w32_gl_load("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = (wglCreateContextAttribsARB_func*)w32_gl_load("wglCreateContextAttribsARB");

        wglMakeCurrent(bootstrap_dc, NULL);
        wglDeleteContext(bootstrap_context);
        ReleaseDC(bootstrap_window, bootstrap_dc);
        UnregisterClassW(bootstrap_wnd_class.lpszClassName, win->backend->instance);
        DestroyWindow(bootstrap_window);
    }

    win->backend->win_class = (WNDCLASSW){
        .hInstance = win->backend->instance,
        .lpfnWndProc = w32_window_proc,
        .lpszClassName = L"Magicalbat Window Class",
        .hCursor = LoadCursor(NULL, IDC_ARROW)
    };
    RegisterClassW(&win->backend->win_class);

    RECT win_rect = { 0, 0, width, height };
    AdjustWindowRect(&win_rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    mga_temp scratch = mga_scratch_get(NULL, 0);
    string16 title16 = str16_from_str8(scratch.arena, title);

    win->backend->window = CreateWindowW(
        win->backend->win_class.lpszClassName,
        title16.str,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        win_rect.right - win_rect.left, win_rect.bottom - win_rect.top,
        NULL, NULL, win->backend->instance, NULL
    );

    mga_scratch_release(scratch);

    SetPropW(win->backend->window, L"gfx_win", win);

    win->backend->device_context = GetDC(win->backend->window);

    i32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_SWAP_METHOD_ARB,    WGL_SWAP_EXCHANGE_ARB,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     0,
        WGL_STENCIL_BITS_ARB,   0,
        0
    };

    i32 pixel_format = 0;
    u32 num_formats = 0;
    wglChoosePixelFormatARB(win->backend->device_context, pixel_format_attribs, NULL, 1, &pixel_format, &num_formats);

    PIXELFORMATDESCRIPTOR pfd = { 0 };
    DescribePixelFormat(win->backend->device_context, pixel_format, sizeof(pfd), &pfd);
    SetPixelFormat(win->backend->device_context, pixel_format, &pfd);

    i32 context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    win->backend->gl_context = wglCreateContextAttribsARB(win->backend->device_context, NULL, context_attribs);

    #define X(ret, name, args) name = (gl_##name##_func)w32_gl_load(#name);
    #   include "opengl_funcs.h"
    #undef X

    if (w32_opengl_module != NULL) {
        FreeLibrary(w32_opengl_module);
    }

    ShowWindow(win->backend->window, SW_SHOW);
    glViewport(0, 0, win->width, win->height);

    return win;
}
void gfx_win_destroy(gfx_window* win) {
    wglMakeCurrent(win->backend->device_context, NULL);
    wglDeleteContext(win->backend->gl_context);
    ReleaseDC(win->backend->window, win->backend->device_context);
    UnregisterClassW(win->backend->win_class.lpszClassName, win->backend->instance);
    DestroyWindow(win->backend->window);
}

void gfx_win_process_events(gfx_window* win) {
    UNUSED(win);

    MSG msg = { 0 };
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void gfx_win_make_current(gfx_window* win) {
    wglMakeCurrent(win->backend->device_context, win->backend->gl_context);
}
void gfx_win_clear(gfx_window* win) {
    UNUSED(win);

    glClear(GL_COLOR_BUFFER_BIT);
}
void gfx_win_swap_buffers(gfx_window* win) {
    SwapBuffers(win->backend->device_context);
}

static LRESULT CALLBACK w32_window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    gfx_window* win = GetPropW(hWnd, L"gfx_win");
    
    switch (uMsg) {
        case WM_SIZE: {
            u32 width = (u32)LOWORD(lParam);
            u32 height = (u32)HIWORD(lParam);

            win->width = width;
            win->height = height;

            glViewport(0, 0, width, height);
        } break;

        case WM_CLOSE: {
            win->should_close = true;
        } break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions
static void* w32_gl_load(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        p = (void*)GetProcAddress(w32_opengl_module, name);
    }
    return p;
}
