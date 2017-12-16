#include "scheduling_simulator.h"

int main()
{
	printf("start\n");

	init_singal_handle();
	hw_task_create("task_t");
	// while (1) {
	//     command_handler();
	// }

	printf("finished\n");
	return 0;
}

void init_singal_handle()
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigaction(SIGTSTP, &act, NULL);
}

void command_handler()
{
	printf("$ ");
	char command[20];
	fgets(command, 20, stdin);
	switch (command[0]) {
	case 'a':
		s_add(get_argv(command, 1), get_argv(command, 3)[0]);
		break;
	case 'r':
		s_remove(atoi(get_argv(command, 1)));
		break;
	case 's':
		s_start();
		break;
	case 'p':
		s_ps();
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

void s_add(const char *t_n, const char t_q)
{
	printf("name: %s\ntime: %c\n", t_n, t_q);
}

void s_remove(const int pid)
{
	printf("pid: %d\n", pid);
}

void s_start()
{
	printf("simulating...\n");
	while (1) {
		printf("0");
	}
}

void s_ps()
{
	printf("ps\n");
}

void handler(int signum)
{
	if (signum == SIGTSTP) {
		printf("ctrl-z\n");
		char c;
		scanf("%c", &c);
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
	char stack[1024 * 128];
	ucontext_t task;

	getcontext(&task);
	task.uc_stack.ss_sp = stack;
	task.uc_stack.ss_size = sizeof(stack);
	task.uc_stack.ss_flags = 0;
	getcontext(&_main);
	_main.uc_stack.ss_sp = stack;
	_main.uc_stack.ss_size = sizeof(stack);
	_main.uc_stack.ss_flags = 0;
	makecontext(&_main, (void (*)(void))main, 0);
	task.uc_link = &_main;

	if (strcmp(task_name, "task1") == 0) {
		makecontext(&task, (void (*)(void))task1, 0);
	} else if (strcmp(task_name, "task2") == 0) {
		makecontext(&task, (void (*)(void))task2, 0);
	} else if (strcmp(task_name, "task3") == 0) {
		makecontext(&task, (void (*)(void))task3, 0);
	} else if (strcmp(task_name, "task4") == 0) {
		makecontext(&task, (void (*)(void))task4, 0);
	} else if (strcmp(task_name, "task5") == 0) {
		makecontext(&task, (void (*)(void))task5, 0);
	} else if (strcmp(task_name, "task6") == 0) {
		makecontext(&task, (void (*)(void))task6, 0);
	} else if (strcmp(task_name, "task_t") == 0) {
		makecontext(&task, (void (*)(void))task_t, 0);
	} else {
		return -1;
	}
	// setcontext(&task);
	swapcontext(&_main, &task);
	return 0; // the pid of created task name
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
 * Insert a node which cantains a mail_t into queue at head.
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
 * Remove a node which cantains a mail_t from queue at tail.
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

void task_t(void)
{
	printf("test~\n");
}
