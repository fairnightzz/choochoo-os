#include "task_descriptor.h"
#include "addrspace.h"
#include "kalloc.h"
#include "stdlib.h"

typedef struct TaskList TaskList;

static TaskList task_list;
static int current_task;

void task_init()
{
    current_task = 0; // uninitialized
    task_list = (TaskList){
        .next_tid = 1,
        .tasks = {0}};
    slab_set_block_size(TASK, sizeof(TaskDescriptor));
    slab_set_block_size(SEND_STATE, sizeof(SendState));
    slab_set_block_size(RECEIVE_STATE, sizeof(ReceiveState));
}

int create_task(uint32_t priority, void (*entrypoint)())
{
    if (task_list.next_tid == 128)
    {
        return -2;
    }

    addrspace addrspace = pagetable_createpage();

    if (addrspace.base == 0 && addrspace.stackbase == 0)
    {
        return -2;
    }

    SwitchFrame *sf = slab_alloc(SWITCH_FRAME);
    if (sf == 0)
    {
        pagetable_deletepage(&addrspace);
        return -2;
    }

    TaskDescriptor *new_task = slab_alloc(TASK);
    if (new_task == 0)
    {
        pagetable_deletepage(&addrspace);
        slab_free(new_task, TASK);
        return -2;
    }

    int new_task_id = (task_list.next_tid)++;

    *sf = switchframe_new(addrspace.stackbase, entrypoint);
    *new_task = (TaskDescriptor){
        .tid = new_task_id,
        .pTid = current_task,
        .status = READY,
        .pri = priority,
        .addrspace = addrspace,
        .switch_frame = sf};
    task_list.tasks[new_task_id] = new_task;

    return new_task_id;
}

TaskDescriptor *get_task(int tid)
{
    if (0 < tid && tid < TASK_SIZE && task_list.tasks[tid] == 0)
    {
        LOG_WARN("getting invalid tid %d", tid);
        return 0;
    }
    return task_list.tasks[tid];
}

void set_current_task(int tid)
{
    if (task_list.tasks[current_task] != 0)
    {
        get_task(current_task)->status = READY;
    }

    get_task(tid)->status = RUNNING;
    current_task = tid;
}

int get_current_task_id()
{
    return current_task;
}

TaskDescriptor *get_current_task()
{
    return get_task(get_current_task_id());
}

void delete_task(int tid)
{
    TaskDescriptor *task = get_task(tid);
    task->status = FINISHED;

    slab_free(task->switch_frame, SWITCH_FRAME);
    pagetable_deletepage(&task->addrspace);
    slab_free(task, TASK);
    task_list.tasks[tid] = 0;
}

void set_task_status(TaskDescriptor *task, TaskStatus status) {
  task->status = status;
}