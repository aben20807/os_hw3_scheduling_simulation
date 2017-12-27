#include "scheduling_simulator.h"

int main()
{
	// printf("start\n");

	pid_count = 1;
	initq(&ready_queue);
	init_context();
	is_simulating = false;
	is_ctrlz = false;
	is_having_now = false;

	signal(SIGTSTP, signal_handler);
	signal(SIGALRM, signal_handler);

	sched_add("task_t", 'L');
	sched_add("task_tt", 'S');

	// ready_queue->display(ready_queue);
	command_handler();

	printf("finished\n");
	return 0;
}

void init_context()
{
	getcontext(&shell_ctx);
	shell_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                                MAP_PRIVATE | MAP_ANON, -1, 0);
	shell_ctx.uc_stack.ss_size = SIGSTKSZ;
	shell_ctx.uc_stack.ss_flags = 0;
	makecontext(&shell_ctx, command_handler, 0);

	getcontext(&sched_ctx);
	sched_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                                MAP_PRIVATE | MAP_ANON, -1, 0);
	sched_ctx.uc_stack.ss_size = SIGSTKSZ;
	sched_ctx.uc_stack.ss_flags = 0;
	makecontext(&sched_ctx, scheduler, 0);

	getcontext(&sighd_ctx);
	sighd_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                                MAP_PRIVATE | MAP_ANON, -1, 0);
	sighd_ctx.uc_stack.ss_size = SIGSTKSZ;
	sighd_ctx.uc_stack.ss_flags = 0;
	makecontext(&sighd_ctx, signal_handler, 0);
}

void command_handler()
{
	while (1) {
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
		case 'e':
			deleq(&ready_queue);
			goto end;
			break;
		default:
			printf("ERROR COMMAND\n");
		}
	}
end:
	;
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
	// task.uc_link = &main_ctx; // TODO ternumated

	if (strcmp(t_n, "task1") == 0) {
		makecontext(&task, task1, 0);
	} else if (strcmp(t_n, "task2") == 0) {
		makecontext(&task, task2, 0);
	} else if (strcmp(t_n, "task3") == 0) {
		makecontext(&task, task3, 0);
	} else if (strcmp(t_n, "task4") == 0) {
		makecontext(&task, task4, 0);
	} else if (strcmp(t_n, "task5") == 0) {
		makecontext(&task, task5, 0);
	} else if (strcmp(t_n, "task6") == 0) {
		makecontext(&task, task6, 0);
	} else if (strcmp(t_n, "task_t") == 0) {
		makecontext(&task, task_t, 0);
	} else if (strcmp(t_n, "task_tt") == 0) {
		makecontext(&task, task_tt, 0);
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
	if (is_ctrlz) {
		is_ctrlz = false;
		is_simulating = true;
		setcontext(&sched_ctx);
	} else {
		setcontext(&sched_ctx);
	}
}

void sched_ps()
{
	// TODO other queues
	printf("ps\n");
	if (ready_queue == NULL || ready_queue->size(ready_queue) == 0) {
		return;
	}
	if (is_having_now) {
		printf("%d\t%s\t%s\t%d\n",
		       now_pcb->pid,
		       now_pcb->name,
		       get_pcb_state(now_pcb->state),
		       now_pcb->q_t);
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
	if (signum == SIGTSTP) {
		is_ctrlz = true;
		is_simulating = false;
		printf("ctrl-z\n");
		swapcontext(&now_ctx, &shell_ctx);
	} else if (signum == SIGALRM) {
		is_simulating = true;
		if (!is_ctrlz)
			swapcontext(&now_pcb->ctx, &sched_ctx);
	}
}

void scheduler()
{
	if (is_simulating) {
		printf("enq\n");
		now_pcb->state = TASK_READY;
		gettimeofday(&now_pcb->t_in, NULL);
		ready_queue->enq(ready_queue, create_node(now_pcb));
	}
	while (ready_queue != NULL && ready_queue->size(ready_queue) != 0) {
		printf("deq\n");
		now_pcb = ready_queue->deq(ready_queue)->pcb;
		struct timeval t_out;
		gettimeofday(&t_out, NULL);
		now_pcb->q_t += ((t_out.tv_usec - now_pcb->t_in.tv_usec) / 1000 + 1000) % 1000;
		// printf("time: %ld - %ld = %ld\n", t_out.tv_usec / 1000,
		// now_pcb->t_in.tv_usec / 1000,
		// ((t_out.tv_usec - now_pcb->t_in.tv_usec) / 1000 + 1000) % 1000);
		is_having_now = true;
		now_pcb->state = TASK_RUNNING;
		/*timer*/
		memset(&it, 0, sizeof it);
		// it.it_value.tv_sec = ((now_pcb->t_q == 'S') ? 1 : 2); // TODO ms
		it.it_value.tv_usec = ((now_pcb->t_q == 'S') ? 10000 : 20000); // TODO ms
		// it.it_interval.tv_usec = ((now_pcb->t_q == 'S') ? 10000 : 20000); // TODO ms
		if (setitimer(ITIMER_REAL, &it, 0)) {
			perror("setitimer");
			exit(1);
		}
		ucontext_t gg_ctx;
		swapcontext(&gg_ctx, &now_pcb->ctx);
	}
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

void initq(Queue **q_ptr)
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

void deleq(Queue **q_ptr)
{
	if (*q_ptr == NULL || (*q_ptr)->size(*q_ptr) == 0) {
		return;
	}
	node *curr = (*q_ptr)->head;
	while (curr != NULL) {
		node *tmp = curr;
		(*q_ptr)->count--;
		curr = curr->next;
		FREE(tmp);
	}
	FREE(*q_ptr);
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

PCB *create_pcb(const char *name, const char t_q, const ucontext_t ctx)
{
	PCB *tmp = NULL;
	CALLOC(tmp, sizeof(*tmp), 1);
	tmp->pid = pid_count++;
	strncpy(tmp->name, name, sizeof(tmp->name));
	tmp->t_q = t_q;
	tmp->t_l = (t_q == 'S') ? 10 : 20;
	tmp->state = TASK_READY;
	tmp->q_t = 0;
	tmp->ctx = ctx;
	return tmp;
}

void task_t(void)
{
	struct timespec delay = {1, 0};
	for (unsigned int i = 1;; i += 2) {
		printf("odd:%d\n", i);
		// printf("test~\n");
		nanosleep(&delay, 0);
	}
}

void task_tt(void)
{
	struct timespec delay = {1, 0};
	for (unsigned int i = 2;; i += 2) {
		printf("eve:%d\n", i);
		// printf("test~\n");
		nanosleep(&delay, 0);
	}
}
