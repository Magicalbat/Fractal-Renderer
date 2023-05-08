#ifndef OS_THREAD_POOL_H
#define OS_THREAD_POOL_H

#include "base/base.h"

// TODO: exit better from threads

typedef struct _thread_pool thread_pool;

typedef void (thread_func)(void*);
typedef struct {
    thread_func* func;
    void* arg;
} thread_task;

thread_pool* thread_pool_create(mg_arena* arena, u32 num_threads, u32 max_tasks);
void thread_pool_destroy(thread_pool* tp);

void thread_pool_add_task(thread_pool* tp, thread_task task);
void thread_pool_wait(thread_pool* tp);

#endif // OS_THREAD_POOL_H
