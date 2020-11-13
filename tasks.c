#include "tasks.h"

/* ************************************************************************** */

typedef struct task {
    system_time_t lastAttempt;
    system_time_t cooldown;
    void (*task_func)(void);
} task_t;

#define MAX_NUM_OF_TASKS 10

typedef struct  {
    task_t taskList[MAX_NUM_OF_TASKS];
    uint8_t numOfTasks;
} task_registry_t;

static task_registry_t registry;

/* ************************************************************************** */

void task_manager_init(void) {
    //
    registry.numOfTasks = 0;
}

uint8_t register_task(void (*task_func)(void), system_time_t cooldown) {
    //
    registry.taskList[registry.numOfTasks].task_func = task_func;
    registry.taskList[registry.numOfTasks].cooldown = cooldown;
    
    return registry.numOfTasks++;
}

bool check_task(uint8_t taskID) {
    task_t task = registry.taskList[taskID];

    if (time_since(task.lastAttempt) < task.cooldown) {
        return false;
    }
    task.lastAttempt = get_current_time();

    task.task_func();

    return true;
}
