#include <stdio.h>
#include <math.h>

#include "base/base.h"
#include "math/math_bigfloat.h"
#include "os/os_thread_pool.h"
#include "math/math_complex.h"

#if 0

#include "gmp.h"
#include <stdlib.h>

typedef struct {
    bigfloat r, i;
} bcx;

typedef struct {
    mpf_t r, i;
} gcx;

void mga_err(mga_error err);
int main(void) {
    mga_desc desc = {
        .desired_max_size = MGA_MiB(16),
        .desired_block_size = MGA_KiB(256),
        .error_callback = mga_err
    };
    mg_arena* perm_arena = mga_create(&desc);

#if 0

    bigfloat a = bf_from_str(perm_arena, STR8("0.f5c28f5c28f5c23d70a3d70a"), 16, 8);
    bigfloat b = bf_from_f64(perm_arena, -1.0, 8);

    //bigfloat correct_ret = bf_from_str(perm_arena, STR8("-0.0a3d70a3d70a4"), 16, 8);

    a.limbs[3] = 4123468604;

    bf_add(&a, &a, &b);

    bf_print(&a, 16);

#else 
    
    complexd c = {
        0.1, 0
    };
    complexd z = { 0 };

    bcx bz = {
        bf_create(perm_arena, 8),
        bf_create(perm_arena, 8)
    };
    bcx bc = {
        bf_from_f64(perm_arena, c.r, 8),
        bf_from_f64(perm_arena, c.i, 8),
    };
    bcx btmp = {
        bf_create(perm_arena, 8),
        bf_create(perm_arena, 8)
    };

    /*gcx gz = { 0 };
    mpf_init_set_d(gz.r, 0);
    mpf_init_set_d(gz.i, 0);
    
    gcx gc = { 0 };
    mpf_init_set_d(gc.r, c.r);
    mpf_init_set_d(gc.i, c.i);

    gcx gtmp = { 0 };
    mpf_init_set_d(gtmp.r, 0);
    mpf_init_set_d(gtmp.i, 0);*/

    bigfloat p = bf_create(perm_arena, 8);

    complexd tmp = { 0 };
    for (u32 i = 0; i < 1; i++) {
        bf_set(&btmp.r, &bz.r);
        bf_set(&btmp.i, &bz.i);
        
        tmp.r = z.r;
        tmp.i = z.i;

        z.r = z.r * z.r;
        z.i = z.i * z.i;
        z.r = z.r - z.i;
        
        bf_mul(&bz.r, &btmp.r, &btmp.r);
        bf_mul(&bz.i, &btmp.i, &btmp.i);
        bf_sub(&bz.r, &bz.r, &bz.i);

        tmp.i = tmp.r * tmp.i;
        z.i = tmp.i + tmp.i;
        
        bf_mul(&btmp.i, &btmp.r, &btmp.i);
        bf_add(&bz.i, &btmp.i, &btmp.i);

        z.r += c.r;
        z.i += c.i;

        bf_add(&bz.r, &bz.r, &bc.r);
        bf_add(&bz.i, &bz.i, &bc.i);

        bf_set_f64(&p, z.r);
        printf("z.r : "); bf_print(&p, 16);
        printf("bz.r: "); bf_print(&bz.r, 16);
        
        bf_set_f64(&p, z.i);
        printf("z.i : "); bf_print(&p, 16);
        printf("bz.i: "); bf_print(&bz.i, 16);
        
        printf("\n=====================================\n\n");
    }

#if 1
    for (u32 i = 0; i < 1; i++) {
        bf_set(&btmp.r, &bz.r);
        bf_set(&btmp.i, &bz.i);
        
        tmp.r = z.r;
        tmp.i = z.i;

        z.r = z.r * z.r;
        bf_set_f64(&p, z.r);
        printf("1 f: "); bf_print(&p, 16);
        z.i = z.i * z.i;
        bf_set_f64(&p, z.i);
        printf("2 f: "); bf_print(&p, 16);
        z.r = z.r - z.i;
        bf_set_f64(&p, z.r);
        printf("3 f: "); bf_print(&p, 16);
       
        bf_mul(&bz.r, &btmp.r, &btmp.r);
        printf("1 b: "); bf_print(&bz.r, 16);
        bf_mul(&bz.i, &btmp.i, &btmp.i);
        printf("2 b: "); bf_print(&bz.i, 16);
        bf_sub(&bz.r, &bz.r, &bz.i);
        printf("3 b: "); bf_print(&bz.r, 16);

        printf("\n");

        tmp.i = tmp.r * tmp.i;
        bf_set_f64(&p, tmp.i);
        printf("4 f: "); bf_print(&p, 16);
        z.i = tmp.i + tmp.i;
        bf_set_f64(&p, z.i);
        printf("5 f: "); bf_print(&p, 16);
        
        bf_mul(&btmp.i, &btmp.r, &btmp.i);
        printf("4 b: "); bf_print(&btmp.i, 16);
        bf_add(&bz.i, &btmp.i, &btmp.i);
        printf("5 b: "); bf_print(&bz.i, 16);
        
        printf("\n");
        
        z.r += c.r;
        z.i += c.i;

        bf_add(&bz.r, &bz.r, &bc.r);
        bf_add(&bz.i, &bz.i, &bc.i);

        bf_set_f64(&p, z.r);
        printf("z.r : "); bf_print(&p, 16);
        printf("bz.r: "); bf_print(&bz.r, 16);
        
        bf_set_f64(&p, z.i);
        printf("z.i : "); bf_print(&p, 16);
        printf("bz.i: "); bf_print(&bz.i, 16);

        printf("\n=====================================\n\n");
    }
#endif

#endif
   
    mga_destroy(perm_arena);

    return 0;
}

void mga_err(mga_error err) {
    printf("MGA ERROR %d: %s", err.code, err.msg);
}

#else

#include "gfx/gfx.h"

#include "math/math_vec.h"

#if defined(PLATFORM_WIN32)
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

#define IMG_WIDTH 640
#define IMG_HEIGHT 360

typedef struct {
    bigfloat r, i;
} complex_bf;

typedef struct {
    f64 x, y, w, h;
} rect64;

typedef struct {
    u8 r, g, b, a;
} pixel8;

typedef struct {
    pixel8* out;
    u32 img_width;
    u32 img_height;
    u32 start_y;
    u32 height;
    complex_bf* complex_dim;
    complex_bf* complex_center;
    u32 iterations;
} mandelbrot_args;

void render_mandelbrot_section(void* void_args) {
    mandelbrot_args* args = (mandelbrot_args*)void_args;

    mga_temp scratch = mga_scratch_get(NULL, 0);

    complex_bf z = {
        .r = bf_create(scratch.arena, args->complex_center->r.prec),
        .i = bf_create(scratch.arena, args->complex_center->i.prec)
    };
    complex_bf c = {
        .r = bf_create(scratch.arena, z.r.prec),
        .i = bf_create(scratch.arena, z.i.prec)
    };

    complex_bf temp = {
        .r = bf_create(scratch.arena, z.r.prec),
        .i = bf_create(scratch.arena, z.i.prec)
    };

    //complexd temp_dim = {
    //    4.0, 4.0 * 9.0 / 16.0
    //};

    bigfloat four = bf_from_f64(scratch.arena, 4.0, 3);
    
    for (u32 y = args->start_y; y < args->start_y + args->height; y++) {
        for (u32 x = 0; x < args->img_width; x++) {
            bf_set_zero(&z.r);
            bf_set_zero(&z.i);

            /*complexd c = {
                (((f64)x / (f64)args->img_width) - 0.5) * args->complex_dim.r + args->complex_center.r,
                (((f64)y / (f64)args->img_height) - 0.5) * args->complex_dim.i + args->complex_center.i
            };*/
            bf_set_f64(&c.r, (f64)x / (f64)args->img_width - 0.5);
            bf_mul(&c.r, &c.r, &args->complex_dim->r);
            bf_add(&c.r, &c.r, &args->complex_center->r);
            bf_set_f64(&c.i, (f64)y / (f64)args->img_height - 0.5);
            bf_mul(&c.i, &c.i, &args->complex_dim->i);
            bf_add(&c.i, &c.i, &args->complex_center->i);

            f32 n = (f32)args->iterations - 1.0;

            for (u32 i = 0; i < args->iterations; i++) {
                // z * z = (z.r * z.r - z.i * z.i, z.r * z.i + z.i * z.r);
                //z = complexd_add(complexd_mul(z, z), c);

                bf_set(&temp.r, &z.r);
                bf_set(&temp.i, &z.i);

                bf_mul(&z.r, &temp.r, &temp.r);
                bf_mul(&z.i, &temp.i, &temp.i);
                bf_sub(&z.r, &z.r, &z.i);

                bf_mul(&temp.r, &temp.r, &temp.i);
                bf_add(&z.i, &temp.r, &temp.r);

                bf_add(&z.r, &z.r, &c.r);
                bf_add(&z.i, &z.i, &c.i);

                bf_mul(&temp.r, &z.r, &z.r);
                bf_mul(&temp.i, &z.i, &z.i);
                bf_add(&temp.r, &temp.r, &temp.i);

                if (bf_cmp(&temp.r, &four) > 0) {
                    n = (f32)i;
                    break;
                }
            }

            u32 j = x + y * args->img_width;
            if (n == (f32)args->iterations - 1.0) {
                args->out[j] = (pixel8){ 0, 0, 0, 255 };
            } else {
                args->out[j] = (pixel8){
                    .r = (u8)((sinf(0.1 * n) * 0.5f + 0.5f) * 255.0f),
                    .g = (u8)((sinf(0.1 * n + 4.188) * 0.5f + 0.5f) * 255.0f),
                    .b = (u8)((sinf(0.1 * n + 2.904) * 0.5f + 0.5f) * 255.0f),
                    .a = 255
                };
                //u32 col = (u32)((n / args->iterations) * 254.0) + 1;
                //args->out[j] = (pixel8){ col, col, col, 255 };
            }
        }
    }

    mga_temp_end(scratch);
}

#define NUM_THREADS 8
static thread_pool* tp = NULL;
void render_mandelbrot(pixel8* out, u32 img_width, u32 img_height, complex_bf* complex_dim, complex_bf* complex_center, u32 iterations) {
    mga_temp scratch = mga_scratch_get(NULL, 0);

    u32 y_step = img_height / NUM_THREADS;
    
    for (u32 i = 0; i < NUM_THREADS; i++) {
        mandelbrot_args* args = MGA_PUSH_ZERO_STRUCT(scratch.arena, mandelbrot_args);

        complex_bf* temp_complex_dim = MGA_PUSH_ZERO_STRUCT(scratch.arena, complex_bf);
        temp_complex_dim->r =  bf_copy(scratch.arena, &complex_dim->r);
        temp_complex_dim->i =  bf_copy(scratch.arena, &complex_dim->i);

        complex_bf* temp_complex_center = MGA_PUSH_ZERO_STRUCT(scratch.arena, complex_bf);
        temp_complex_center->r =  bf_copy(scratch.arena, &complex_center->r);
        temp_complex_center->i =  bf_copy(scratch.arena, &complex_center->i);

        *args = (mandelbrot_args){
            .out = out,
            .img_width = img_width,
            .img_height = img_height,
            .start_y = y_step * i,
            .height = y_step,
            .complex_dim = temp_complex_dim,
            .complex_center = temp_complex_center,
            .iterations = iterations
        };

        render_mandelbrot_section(args);
        /*thread_pool_add_task(
            tp,
            (thread_task){
                .func = render_mandelbrot_section,
                .arg = args
            }
        );*/
    }

    //thread_pool_wait(tp);

    mga_scratch_release(scratch);
}

void draw(gfx_window* win);

void mga_err(mga_error err) {
    printf("MGA ERROR %d: %s", err.code, err.msg);
}

static u32 vertex_buffer, vertex_array;
static struct {
    u32 shader, texture;
} gl_fract = { 0 };
static struct {
    u32 shader, scale_loc, offset_loc;
} gl_rect = { 0 };

static vec2f init_rect_pos = { 0 };
static rect64 mouse_norm_rect(gfx_window* win) {
    vec2f p0 = init_rect_pos;
    vec2f p1 = win->mouse_pos;
    if (p1.x < p0.x) {
        vec2f temp = p0;
        p0 = p1;
        p1 = temp;
    }

    rect64 rect = {
        p0.x, p0.y,
        p1.x - p0.x,
        (f64)(p1.x - p0.x) * (9.0 / 16.0)
    };
    rect.x /= (f64)win->width;
    rect.w /= (f64)win->width;
    rect.y /= (f64)win->height;
    rect.h /= (f64)win->height;

    return rect;
}

#define MANDEL_PREC 4

int main(void) {
    mga_desc desc = {
        .desired_max_size = MGA_MiB(16),
        .desired_block_size = MGA_KiB(256),
        .error_callback = mga_err
    };
    mg_arena* perm_arena = mga_create(&desc);

    fpng_init();

    gfx_window* win = gfx_win_create(perm_arena, WIDTH, HEIGHT, STR8("Fractal Renderer"));

    tp = thread_pool_create(perm_arena, NUM_THREADS, 128);

    pixel8* screen = MGA_PUSH_ZERO_ARRAY(perm_arena, pixel8, IMG_WIDTH * IMG_HEIGHT);
    for (u32 i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
        screen[i].a = 255;
    }

    const char* fract_vert_source = ""
        "#version 330 core\n"
        "layout (location = 0) in vec2 a_pos;"
        "layout (location = 1) in vec2 a_uv;"
        "out vec2 uv;"
        "void main() {"
        "   uv = a_uv;"
        "   gl_Position = vec4(a_pos, 0, 1);"
        "}";
    const char* fract_frag_source = ""
        "#version 330 core\n"
        "layout (location = 0) out vec4 out_col;"
        "uniform sampler2D u_texture;"
        "in vec2 uv;"
        "void main() {"
        "    vec4 sample = texture(u_texture, uv);"
        "    out_col = sample;"
        "}";
    
    const char* rect_vert_source = ""
        "#version 330 core\n"
        "layout (location = 0) in vec2 a_pos;"
        "layout (location = 1) in vec2 a_uv;"
        "uniform vec2 u_scale;"
        "uniform vec2 u_offset;"
        "out vec2 uv;"
        "flat out vec2 scale;"
        "void main() {"
        "    uv = a_uv;"
        "    scale = u_scale;"
        "   gl_Position = vec4(a_pos * u_scale + u_offset, 0, 1);"
        "}";
    const char* rect_frag_source = ""
        "#version 330 core\n"
        "layout (location = 0) out vec4 out_col;"
        "in vec2 uv;"
        "flat in vec2 scale;"
        "void main() {"
        "    vec2 border = vec2(0.002 / scale.x, 0.003 / scale.y);"
        "    float alpha = step(uv.x, border.x);"
        "    alpha = max(alpha, step(1.0 - border.x, uv.x));"
        "    alpha = max(alpha, step(uv.y, border.y));"
        "    alpha = max(alpha, step(1.0 - border.y, uv.y));"
        "    out_col = vec4(vec3(1.), alpha);"
        "}";

    f32 verts[4 * 6] = {
        -1.0f, -1.0f,   0.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 0.0f,

        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    vertex_buffer = glh_create_buffer(
        GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW
    );
    
    gl_fract.shader = glh_create_shader(fract_vert_source, fract_frag_source);
    
    gl_rect.shader = glh_create_shader(rect_vert_source, rect_frag_source);
    gl_rect.scale_loc = glGetUniformLocation(gl_rect.shader, "u_scale");
    gl_rect.offset_loc = glGetUniformLocation(gl_rect.shader, "u_offset");

    glGenTextures(1, &gl_fract.texture);
    glBindTexture(GL_TEXTURE_2D, gl_fract.texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, IMG_WIDTH, IMG_HEIGHT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    glClearColor(0.45f, 0.65f, 0.77f, 1.0f);

    complex_bf complex_dim = {
        bf_from_f64(perm_arena, 4.0, MANDEL_PREC),
        bf_from_f64(perm_arena, 4.0 * 9.0 / 16.0, MANDEL_PREC)
    };
    complex_bf complex_center = { 
        bf_create(perm_arena, MANDEL_PREC),
        bf_create(perm_arena, MANDEL_PREC)
    };
    complex_bf temp_bf = {
        bf_create(perm_arena, MANDEL_PREC),
        bf_create(perm_arena, MANDEL_PREC)
    };

    u32 iterations = 64;

    render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, &complex_dim, &complex_center, iterations);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);

    while (!win->should_close) {
        gfx_win_process_events(win);

        if (win->mouse_buttons[0] && !win->prev_mouse_buttons[0]) {
            init_rect_pos = win->mouse_pos;
        }

        if (!win->mouse_buttons[0] && win->prev_mouse_buttons[0]) {
            rect64 rect = mouse_norm_rect(win);
            if (rect.x == 0) rect.x = 1;
            if (rect.y == 0) rect.y = 1;
            vec2d center = {
                rect.x + rect.w * 0.5,
                rect.y + rect.h * 0.5
            };

            //complex_center.r += (center.x - 0.5) * complex_dim.r;
            //complex_center.i += (center.y - 0.5) * complex_dim.i;
            bf_set_f64(&temp_bf.r, center.x - 0.5);
            bf_set_f64(&temp_bf.i, center.y - 0.5);
            bf_mul(&temp_bf.r, &temp_bf.r, &complex_dim.r);
            bf_mul(&temp_bf.i, &temp_bf.i, &complex_dim.i);
            bf_add(&complex_center.r, &temp_bf.r, &complex_center.r);
            bf_add(&complex_center.i, &temp_bf.i, &complex_center.i);

            bf_set_f64(&temp_bf.r, rect.w);
            bf_mul(&complex_dim.r, &complex_dim.r, &temp_bf.r);
            bf_mul(&complex_dim.i, &complex_dim.i, &temp_bf.r);
            //complex_dim = complexd_scale(complex_dim, rect.w);

            iterations += 64;
            
            //printf("dim: %f %f, center: %f %f, iters: %u\n", complex_dim.r, complex_dim.i, complex_center.r, complex_center.i, iterations);

            render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, &complex_dim, &complex_center, iterations);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);
        }

        if (win->mouse_buttons[2] && !win->prev_mouse_buttons[2]) {
            printf("saving images\n");

            mga_temp temp = mga_temp_begin(perm_arena);
            
            bf_set_f64(&temp_bf.r, 4.0);
            bf_set_f64(&temp_bf.i, 1.5);

            u32 i = 0;
            b32 done = false;
            while (!done) {
                if (bf_cmp(&complex_dim.r, &temp_bf.r) > 0)
                    done = true;
                
                render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, &complex_dim, &complex_center, 1024);

                //complex_dim = complexd_scale(complex_dim, 1.5);
                bf_mul(&complex_dim.r, &complex_dim.r, &temp_bf.i);
                bf_mul(&complex_dim.i, &complex_dim.i, &temp_bf.i);
                
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
                
                #ifdef PLATFORM_WIN32
                FILE* f = NULL;
                fopen_s(&f, file_path, "wb");
                #else
                FILE* f = fopen(file_path, "wb");
                #endif
                fwrite(out.str, 1, out.size, f);
                fclose(f);
                
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);
                draw(win);

                mga_temp_end(temp);
                
                printf("image %d\n", i - 1);
            }
            i--;

            char file1[256] = { 0 };
            char file2[256] = { 0 };
            for (u32 j = 0; j < i / 2 + 1; j++) {
                sprintf(file1, "out/img_%.4u.png", j);
                sprintf(file2, "out/img_%.4u.png", i - j);
                
                rename(file1, "out/temp.png");
                rename(file2, file1);
                rename("out/temp.png", file2);
            }

            printf("done saving images\n");

            render_mandelbrot(screen, IMG_WIDTH, IMG_HEIGHT, &complex_dim, &complex_center, 512);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screen);
            draw(win);
        }

        draw(win);
        
        #if defined(PLATFORM_WIN32)
            Sleep(16);
        #elif defined(PLATFORM_LINUX)
            usleep(16000);
        #endif
    }

    glDeleteTextures(1, &gl_fract.texture);
    glDeleteProgram(gl_fract.shader);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);

    gfx_win_destroy(win);

    thread_pool_destroy(tp);

    mga_destroy(perm_arena);

    return 0;
}

void draw(gfx_window* win) {
    gfx_win_clear(win);

    glUseProgram(gl_fract.shader);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl_fract.texture);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(sizeof(f32) * 2));

    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (win->mouse_buttons[0]) {
        glUseProgram(gl_rect.shader);

        rect64 mouse_rect = mouse_norm_rect(win);
        vec2d center = {
            mouse_rect.x + mouse_rect.w * 0.5,
            mouse_rect.y + mouse_rect.h * 0.5
        };

        glUniform2f(gl_rect.scale_loc, mouse_rect.w, mouse_rect.h);
        glUniform2f(
            gl_rect.offset_loc,
            center.x * 2.0f - 1.0f,
            -center.y * 2.0f + 1.0f
        );
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    gfx_win_swap_buffers(win);
}
#endif
