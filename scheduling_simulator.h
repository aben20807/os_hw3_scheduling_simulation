#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "task.h"

enum TASK_STATE {
	TASK_RUNNING,
	TASK_READY,
	TASK_WAITING,
	TASK_TERMINATED
};

#define MALLOC(p, s) \
    if(!((p) = malloc(s))){ \
        fprintf(stderr, "insufficient memory"); \
        exit(EXIT_FAILURE); \
    }
#define CALLOC(p, l, s) \
    if(!((p) = calloc(l, s))){ \
        fprintf(stderr, "insufficient memory"); \
        exit(EXIT_FAILURE); \
    }
#define FREE(p) \
    if(p != NULL){ \
        free(p); \
        p = NULL; \
    }

typedef struct PCB {
	int pid;
	char name[10];
	char t_q;       // time quantum
	int state;
	int q_t;        // queueing time
	ucontext_t context;
} PCB;

/*
 * Queue
 */
typedef struct node {
	struct PCB *pcb;
	struct node *prev;
	struct node *next;
} node;

typedef struct Queue {
	int count;
	node *head;
	node *tail;
	int (*size)();
	bool (*enq)();
	node *(*deq)();
	bool (*display)();
} Queue;

/* Global Variable */
ucontext_t _main;
int pid_count;
Queue *ready_queue;
Queue *waiting_queue;

void init_main_context();
void init_singal_handle();
void command_handler();
char *get_argv(const char *command, const int num);
int sched_add(const char *t_n, const char t_q);
void sched_remove(const int pid);
void sched_start();
void sched_ps();
void handler();
char *get_pcb_state(const int state);

void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);

void task_t(void);

/* Queue Function */
void init(Queue **q_ptr);
int size(Queue *self);
bool enq(Queue *self, node *item);
node *deq(Queue *self);
bool display(Queue *self);
node *create_node(PCB *pcb);
PCB *create_pcb(const char *name, const char t_q, const ucontext_t context);

#endif
