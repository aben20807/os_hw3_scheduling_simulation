#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>

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
	int state;
	long s_t;        // suspend time
	long q_t;        // queueing time
	ucontext_t ctx; // context
	char name[10];
	char t_q;       // time quantum
	struct timeval t_in; // timestamp when enter ready queue
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
ucontext_t now_ctx;
ucontext_t sighd_ctx; // for function signal_handler()
ucontext_t sched_ctx; // for function scheduler()
ucontext_t shell_ctx; // for function command_handler()
ucontext_t terhd_ctx; // for function terminated_handler()
ucontext_t savsu_ctx; // for function save_suspend()
PCB *now_pcb;
int pid_count;
static volatile sig_atomic_t is_simulating;
static volatile sig_atomic_t is_ctrlz;
static volatile sig_atomic_t is_having_now;
static volatile sig_atomic_t is_terminated;
Queue *ready_queue;
Queue *waiting_queue;
Queue *terminated_queue;
struct itimerval it;

void init_context();
void command_handler();
char *get_argv(const char *command, const int num);
int sched_add(const char *t_n, const char t_q);
void sched_remove(const int pid);
void sched_ps();
void signal_handler();
void store_running_task();
void scheduler();
void terminated_handler();
char *get_pcb_state(const int state);
void save_suspend();

void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);

void task_t(void);
void task_tt(void);

/* Queue Function */
void initq(Queue **q_ptr);
void deleq(Queue **q_ptr);
int size(Queue *self);
bool enq(Queue *self, node *item);
node *deq(Queue *self);
bool display(Queue *self);
node *create_node(PCB *pcb);
PCB *create_pcb(const char *name, const char t_q, const ucontext_t context);

#endif
