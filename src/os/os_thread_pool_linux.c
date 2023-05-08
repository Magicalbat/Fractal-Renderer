#include "base/base_defs.h"

#ifdef PLATFORM_LINUX

#include "os_thread_pool.h"

#include <stdio.h>
#include <pthread.h>

typedef struct _thread_pool {
    u32 num_threads;
    pthread_t* threads;

    u32 max_tasks;
    u32 num_tasks;
    thread_task* task_queue;

    pthread_mutex_t mutex;
    pthread_cond_t queue_cond_var;

    u32 num_active;
    pthread_cond_t active_cond_var;
} thread_pool;

static void* linux_thread_start(void* arg) {
    thread_pool* tp = (thread_pool*)arg;
    thread_task task = { 0 };

    while (true) {
        pthread_mutex_lock(&tp->mutex);

        while (tp->num_tasks == 0) {
            pthread_cond_wait(&tp->queue_cond_var, &tp->mutex);
        }

        tp->num_active++;
        task = tp->task_queue[0];
        for (u32 i = 0; i < tp->num_tasks - 1; i++) {
            tp->task_queue[i] = tp->task_queue[i + 1];
        }
        tp->num_tasks--;

        pthread_mutex_unlock(&tp->mutex);

        task.func(task.arg);

        pthread_mutex_lock(&tp->mutex);

        tp->num_active--;
        if (tp->num_active == 0) {
            pthread_cond_signal(&tp->active_cond_var);
        }

        pthread_mutex_unlock(&tp->mutex);
    }

    return NULL;
}

thread_pool* thread_pool_create(mg_arena* arena, u32 num_threads, u32 max_tasks) {
    thread_pool* tp = MGA_PUSH_ZERO_STRUCT(arena, thread_pool);

    tp->max_tasks = max_tasks;
    tp->task_queue = MGA_PUSH_ZERO_ARRAY(arena, thread_task, max_tasks);

    pthread_mutex_init(&tp->mutex, NULL);
    pthread_cond_init(&tp->queue_cond_var, NULL);
    pthread_cond_init(&tp->active_cond_var, NULL);

    tp->num_threads = num_threads;
    tp->threads = MGA_PUSH_ZERO_ARRAY(arena, pthread_t, num_threads);
    for (u32 i = 0; i < num_threads; i++) {
        pthread_create(&tp->threads[i], NULL, linux_thread_start, tp);
    }

    return tp;
}
void thread_pool_destroy(thread_pool* tp) {
    for (u32 i = 0; i < tp->num_threads; i++) {
        pthread_cancel(tp->threads[i]);
    }

    pthread_mutex_destroy(&tp->mutex);
    pthread_cond_destroy(&tp->queue_cond_var);
    pthread_cond_destroy(&tp->active_cond_var);
}

void thread_pool_add_task(thread_pool* tp, thread_task task) {
    pthread_mutex_lock(&tp->mutex);

    if ((u64)tp->num_tasks + 1 >= (u64)tp->max_tasks) {
        pthread_mutex_unlock(&tp->mutex);
        fprintf(stderr, "Thread pool exceeded max tasks\n");
        return;
    }

    tp->task_queue[tp->num_tasks++] = task;

    pthread_mutex_unlock(&tp->mutex);

    pthread_cond_signal(&tp->queue_cond_var);
}
void thread_pool_wait(thread_pool* tp) {
    pthread_mutex_lock(&tp->mutex);

    while (true) {
        if (tp->num_active != 0 || tp->num_tasks != 0) {
            pthread_cond_wait(&tp->active_cond_var, &tp->mutex);
        } else {
            break;
        }
    }

    pthread_mutex_unlock(&tp->mutex);
}

#endif // PLATFORM_LINUX
