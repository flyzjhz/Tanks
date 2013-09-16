#ifndef _CORE_TASK_TASK_H
#define _CORE_TASK_TASK_H

#include <core/common/common.h>
#include <core/list/list.h>

/**
 * sturct task - a task entry
 * @struct list_head : linked list pointer, and it should order by remain_time asc
 * @id : identification, increase
 * @remain_time : time remaining
 * @type : task type
 * @param : a pointer point to a param or more params(struct)
 */
struct task {
    struct list_head list;
    uint16_t id;
    uint16_t remain_time;
    uint8_t type;
    void *param;
};

/**
 * task_head - a task header
 * @head : the first task
 * @count : all task count
 */
struct task_head {
    struct task head;
    uint16_t count;
};

/**
 * init_task - init a task_head struct
 * @*task_head : a pointer point to a struct task_head needing initialized
 */
int init_task(struct task_head *task_head);
/**
 * add_task - insert a task to struct task
 * @struct task : struct task inserted
 */
int add_task(struct task *task);

/**
 * del_task_id - delete a task throught id of struct task
 * @id : task id
 */
int remove_task_id(uint16_t id);
#endif
