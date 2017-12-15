#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

#include "task.h"

enum TASK_STATE {
	TASK_RUNNING,
	TASK_READY,
	TASK_WAITING,
	TASK_TERMINATED
};

#define FREE(p) \
    if(p != NULL){ \
        free(p); \
        p = NULL; \
    }

void init_singal_handle();
void command_handler();
char *get_argv(const char *command, const int num);
void s_add(const char *t_n, const char t_q);
void s_remove(const int pid);
void s_start();
void s_ps();
void handler();

void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);

#endif
