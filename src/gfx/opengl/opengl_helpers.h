#ifndef OPENGL_HELPERS_H
#define OPENGL_HELPERS_H

#include "base/base_defs.h"

u32 glh_create_shader(const char* vertex_source, const char* fragment_source);
u32 glh_create_buffer(u32 buffer_type, u64 size, void* data, u32 draw_type);

#endif // OPENGL_HELPERS_H
