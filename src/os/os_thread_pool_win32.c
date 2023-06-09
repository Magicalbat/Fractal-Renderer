#include "base/base_defs.h"

#ifdef PLATFORM_WIN32

#include "os_thread_pool.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>

// TODO: look into PTP_POOL

typedef struct _thread_pool {
    u32 num_threads;
    HANDLE* threads;

    u32 max_tasks;
    u32 num_tasks;
    thread_task* task_queue;

    CRITICAL_SECTION mutex; // I know that it is not technically a mutex on win32
    CONDITION_VARIABLE queue_cond_var;

    u32 num_active;
    CONDITION_VARIABLE active_cond_var;
} thread_pool;

static DWORD w32_thread_start(void* arg) {
    thread_pool* tp = (thread_pool*)arg;
    thread_task task = { 0 };

    while (true) {
        EnterCriticalSection(&tp->mutex);
        while (tp->num_tasks == 0) {
            SleepConditionVariableCS(&tp->queue_cond_var, &tp->mutex, INFINITE);
        }

        tp->num_active++;
        task = tp->task_queue[0];
        for (u32 i = 0; i < tp->num_tasks - 1; i++) {
            tp->task_queue[i] = tp->task_queue[i + 1];
        }
        tp->num_tasks--;

        LeaveCriticalSection(&tp->mutex);

        task.func(task.arg);

        EnterCriticalSection(&tp->mutex);

        tp->num_active--;
        if (tp->num_active == 0) {
            WakeConditionVariable(&tp->active_cond_var);
        }

        LeaveCriticalSection(&tp->mutex);
    }

    return 0;
}

thread_pool* thread_pool_create(mg_arena* arena, u32 num_threads, u32 max_tasks) {
    thread_pool* tp = MGA_PUSH_ZERO_STRUCT(arena, thread_pool);

    tp->max_tasks = max_tasks;
    tp->task_queue = MGA_PUSH_ZERO_ARRAY(arena, thread_task, max_tasks);

    InitializeCriticalSection(&tp->mutex);
    InitializeConditionVariable(&tp->queue_cond_var);
    InitializeConditionVariable(&tp->active_cond_var);

    tp->num_threads = num_threads;
    tp->threads = MGA_PUSH_ZERO_ARRAY(arena, HANDLE, num_threads);
    for (u32 i = 0; i < num_threads; i++) {
        tp->threads[i] = CreateThread(
            NULL, 0, w32_thread_start, tp, 0, NULL
        );
    }

    return tp;
}
void thread_pool_destroy(thread_pool* tp) {
    for (u32 i = 0; i < tp->num_threads; i++) {
        CloseHandle(tp->threads[i]);
    }

    DeleteCriticalSection(&tp->mutex);
}

void thread_pool_add_task(thread_pool* tp, thread_task task) {
    EnterCriticalSection(&tp->mutex);

    if ((u64)tp->num_tasks + 1 >= (u64)tp->max_tasks) {
        LeaveCriticalSection(&tp->mutex);
        fprintf(stderr, "Thread pool exceeded max tasks\n");
        return;
    }

    tp->task_queue[tp->num_tasks++] = task;

    LeaveCriticalSection(&tp->mutex);
    WakeConditionVariable(&tp->queue_cond_var);
}
void thread_pool_wait(thread_pool* tp) {
    EnterCriticalSection(&tp->mutex);
    while (true) {
        if (tp->num_active != 0 || tp->num_tasks != 0) {
            SleepConditionVariableCS(&tp->active_cond_var, &tp->mutex, INFINITE);
        } else {
            break;
        }
    }
    LeaveCriticalSection(&tp->mutex);

}

#endif // PLATFORM_WIN32
