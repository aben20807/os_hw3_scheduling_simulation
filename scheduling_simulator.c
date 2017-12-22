#include "scheduling_simulator.h"

int main()
{
	printf("start\n");

	pid_count = 1;
	init(&ready_queue);
	init_main_context();
	switch_context = 0;

	/*timer*/
	struct itimerval it;
	memset(&it, 0, sizeof it);
	it.it_interval.tv_sec = 2;
	it.it_value.tv_sec = 2;
	if (setitimer(ITIMER_REAL, &it, 0)) {
		perror("setitimer");
		exit(1);
	}
	signal(SIGTSTP, signal_handler);
	signal(SIGALRM, signal_handler);

	hw_task_create("task_t");
	hw_task_create("task_t");
	hw_task_create("task_t");
	hw_task_create("task_t");
	// node *t = ready_queue->deq(ready_queue);
	// swapcontext(&main_ctx, &t->pcb->context);
	// t = ready_queue->deq(ready_queue);
	// swapcontext(&main_ctx, &t->pcb->context);
	// t = ready_queue->deq(ready_queue);
	// swapcontext(&main_ctx, &t->pcb->context);

	// ready_queue->display(ready_queue);
	while (1) {
		command_handler();
	}

	printf("finished\n");
	return 0;
}

void init_main_context()
{
	getcontext(&main_ctx);
	main_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                               MAP_PRIVATE | MAP_ANON, -1, 0);
	main_ctx.uc_stack.ss_size = SIGSTKSZ;
	main_ctx.uc_stack.ss_flags = 0;
	makecontext(&main_ctx, (void (*)(void))main, 0);
}

void command_handler()
{
	printf("$ ");
	char command[20];
	fgets(command, 20, stdin);
	switch (command[0]) {
	case 'a':
		sched_add(get_argv(command, 1), get_argv(command, 3)[0]);
		break;
	case 'r':
		sched_remove(atoi(get_argv(command, 1)));
		break;
	case 's':
		sched_start();
		break;
	case 'p':
		sched_ps();
		break;
	default:
		printf("ERROR COMMAND\n");
	}
}

char *get_argv(const char *command, const int num)
{
	char delim[] = " ";
	// copy string for split
	char *s = (char *)strndup(command, 20);
	// change '\n' into '\0'
	char *pos;
	if ((pos = strchr(s, '\n')) != NULL)
		* pos = '\0';
	// split
	char *token;
	int argc = 0;
	for (token = strsep(&s, delim); token != NULL; token = strsep(&s, delim)) {
		if (argc == num) {
			return token;
		}
		argc++;
	}
	FREE(s);
	return "\0";
}

int sched_add(const char *t_n, const char t_q)
{
	// printf("name: %s\ntime: %c\n", t_n, t_q);
	ucontext_t task;
	getcontext(&task);
	task.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                           MAP_PRIVATE | MAP_ANON, -1, 0);
	task.uc_stack.ss_size = SIGSTKSZ;
	task.uc_stack.ss_flags = 0;
	task.uc_link = &main_ctx;

	if (strcmp(t_n, "task1") == 0) {
		makecontext(&task, (void (*)(void))task1, 0);
	} else if (strcmp(t_n, "task2") == 0) {
		makecontext(&task, (void (*)(void))task2, 0);
	} else if (strcmp(t_n, "task3") == 0) {
		makecontext(&task, (void (*)(void))task3, 0);
	} else if (strcmp(t_n, "task4") == 0) {
		makecontext(&task, (void (*)(void))task4, 0);
	} else if (strcmp(t_n, "task5") == 0) {
		makecontext(&task, (void (*)(void))task5, 0);
	} else if (strcmp(t_n, "task6") == 0) {
		makecontext(&task, (void (*)(void))task6, 0);
	} else if (strcmp(t_n, "task_t") == 0) {
		makecontext(&task, (void (*)(void))task_t, 0);
	} else {
		return -1;
	}
	PCB *tmp = create_pcb(t_n, t_q, task);
	ready_queue->enq(ready_queue, create_node(tmp));
	return tmp->pid;
}

void sched_remove(const int pid)
{
	printf("pid: %d\n", pid);
}

void sched_start()
{
	printf("simulating...\n");
	while (ready_queue->size(ready_queue) != 0) {
		if (switch_context == 1) {
			switch_context = 0;
			printf("deq\n");
			PCB *tmp_pcb = ready_queue->deq(ready_queue)->pcb;
			ucontext_t tmp_ctx = tmp_pcb->context;
			getcontext(&main_ctx);
			swapcontext(&main_ctx, &tmp_ctx);
		}
	}
}

void sched_ps()
{
	// TODO other queues
	printf("ps\n");
	if (ready_queue == NULL || ready_queue->size(ready_queue) == 0) {
		return;
	}
	node *curr = ready_queue->head;
	while (curr != NULL) {
		printf("%d\t%s\t%s\t%d\n",
		       curr->pcb->pid,
		       curr->pcb->name,
		       get_pcb_state(curr->pcb->state),
		       curr->pcb->q_t);
		curr = curr->next;
	}
}

char *get_pcb_state(const int state)
{
	switch (state) {
	case TASK_RUNNING:
		return "TASK_RUNNING";
	case TASK_READY:
		return "TASK_READY";
	case TASK_WAITING:
		return "TASK_WAITING";
	case TASK_TERMINATED:
		return "TASK_TERMINATED";
	default:
		return "";
	}
}

void signal_handler(int signum)
{
	ucontext_t task;
	getcontext(&task);
	if (signum == SIGTSTP) {
		printf("ctrl-z\n");
	} else if (signum == SIGALRM) {
		// printf("time: %d\n", pid_count++);
		switch_context = 1;
	}
	// swapcontext(&task, &main_ctx);
}

void hw_suspend(int msec_10)
{
	return;
}

void hw_wakeup_pid(int pid)
{
	return;
}

int hw_wakeup_taskname(char *task_name)
{
	return 0;
}

int hw_task_create(char *task_name)
{
	return sched_add(task_name, 'S');
	// return 0; // the pid of created task name
}

void init(Queue **q_ptr)
{
	Queue *q = NULL;
	MALLOC(q, sizeof(Queue));
	q->count = 0;
	q->head = NULL;
	q->tail = NULL;
	q->size = size;
	q->enq = enq;
	q->deq = deq;
	q->display = display;
	*q_ptr = q;
}

int size(Queue *self)
{
	return self->count;
}

/*
 * Insert a node which cantains a PCB into queue at head.
 */
bool enq(Queue *self, node *item)
{
	if ((self == NULL) || (item == NULL)) {
		return false;
	}
	if (self->size(self) == 0) {
		self->head = item;
		self->tail = item;
	} else {
		self->head->prev = item;
		item->next = self->head;
		self->head = item;
	}
	self->count++;
	return true;
}

/*
 * Remove a node which cantains a PCB from queue at tail.
 */
node *deq(Queue *self)
{
	if ((self == NULL) || self->size(self) == 0) {
		// printf("0\n");
		return NULL;
	}
	node *tmp = self->tail;
	if (self->size(self) != 1) {
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	} else {
		self->tail = self->head;
		self->count = 0;
		return tmp;
	}
	self->count--;
	return tmp;
}

/*
 * Display all nodes in queue from head to tail.
 */
bool display(Queue *self)
{
	if (self == NULL || self->size(self) == 0) {
		return false;
	}
	printf("head->\n");
	node *curr = self->head;
	int count = 0;
	while (curr != NULL) {
		printf("%d:\n%d\n%s\n",
		       count++,
		       curr->pcb->pid,
		       curr->pcb->name);
		curr = curr->next;
	}
	printf("<-tail\n");
	return true;
}

node *create_node(PCB *pcb)
{
	node *tmp = NULL;
	CALLOC(tmp, sizeof(*tmp), 1);
	tmp->pcb = pcb;
	tmp->prev = NULL;
	tmp->next = NULL;
	return tmp;
}

PCB *create_pcb(const char *name, const char t_q, const ucontext_t context)
{
	PCB *tmp = NULL;
	CALLOC(tmp, sizeof(*tmp), 1);
	tmp->pid = pid_count++;
	strncpy(tmp->name, name, sizeof(tmp->name));
	tmp->t_q = t_q;
	tmp->state = TASK_READY;
	tmp->q_t = 0;
	tmp->context = context;
	return tmp;
}

void task_t(void)
{
	printf("test~\n");
}
