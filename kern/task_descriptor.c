#include "task_descriptor.h"
#include "addrspace.h"
#include "kalloc.h"
#include "stdlib.h"

typedef struct TaskList TaskList;

static TaskList task_list;
static uint32_t current_task;

void task_init(void) {
    current_task = 1; // uninitialized
    task_list = (TaskList) {
        .next_tid = 1,
        .tasks = {0}
    };
    slab_set_block_size(TASK, sizeof(TaskDescriptor));
}

uint32_t tasktable_create_task(uint32_t priority, void (*entrypoint)())
{
    addrspace addrspace = pagetable_createpage();

    uint32_t new_task_id = (task_list.next_tid)++;

    TaskDescriptor* new_task = slab_alloc(TASK);
    SwitchFrame* sf = slab_alloc(SWITCH_FRAME);
    *sf = switchframe_new(addrspace.stackbase, entrypoint);

    *new_task = (TaskDescriptor) {
        .tid = new_task_id,
        .pTid = 0,
        .status = READY,
        .pri = priority,
        .addrspace = addrspace,
        .switch_frame = sf
    };

    task_list.tasks[new_task_id] = new_task;

    return new_task_id;
}

TaskDescriptor* tasktable_get_task(uint32_t tid) {
    if (task_list.tasks[tid] == 0) LOG_WARN("getting invalid tid %d", tid);
    return task_list.tasks[tid];
}

void
tasktable_set_current_task(uint32_t tid)
{
    tasktable_get_task(current_task)->status = READY;

    tasktable_get_task(tid)->status = RUNNING;
    current_task = tid;
}

uint32_t tasktable_current_task(void) {
    return current_task;
}

void tasktable_delete_task(uint32_t tid) {
    TaskDescriptor* task = tasktable_get_task(tid);
    task->status = FINISHED;

    slab_free(task->switch_frame, SWITCH_FRAME);
    slab_free(task, TASK);
    task_list.tasks[tid] = 0;
}
