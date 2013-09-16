#include <core/task/task.h>

int init_task(struct task_head *task_head){
    memset(task_head, 0, sizeof(*task_head));
}

int add_task(struct task_head *task_head, struct task *task) {
    static uint16_t id = 0;
    task->id = ++id;
    struct task *pos;
    list_for_each_entry_safe(pos, task_head.head, list){
        if(pos->remain_time > task->remain_time){
            break;
        }
    }
    list_add_tail(task, task_head->head.list);
    return 0;
}

int remove_task_id(uint16_t id) {
    struct task *pos;
    list_for_each_entry_safe(pos, task_head.head, list){
        if(pos->id == task->id){
            list_del(pos.list);
        }
    }
}
