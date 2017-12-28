#include "scheduling_simulator.h"

int main()
{
	pid_count = 1;
	initq(&ready_queue);
	initq(&waiting_queue);
	initq(&terminated_queue);
	init_context();
	is_simulating = false;
	is_ctrlz = false;
	is_having_now = false;
	is_terminated = false;

	signal(SIGTSTP, signal_handler);
	signal(SIGALRM, signal_handler);
	command_handler();
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

	getcontext(&terhd_ctx);
	terhd_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                                MAP_PRIVATE | MAP_ANON, -1, 0);
	terhd_ctx.uc_stack.ss_size = SIGSTKSZ;
	terhd_ctx.uc_stack.ss_flags = 0;
	makecontext(&terhd_ctx, terminated_handler, 0);

	getcontext(&savsu_ctx);
	savsu_ctx.uc_stack.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE,
	                                MAP_PRIVATE | MAP_ANON, -1, 0);
	savsu_ctx.uc_stack.ss_size = SIGSTKSZ;
	savsu_ctx.uc_stack.ss_flags = 0;
	makecontext(&savsu_ctx, save_suspend, 0);
}

void command_handler()
{
	ucontext_t gg_ctx;
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
			printf("simulating...\n");
			reset_ready_task_time();
			swapcontext(&gg_ctx, &sched_ctx);
			break;
		case 'p':
			sched_ps();
			break;
		case 'e':
		case 'q':
			deleq(&ready_queue);
			deleq(&terminated_queue);
			deleq(&waiting_queue);
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
	return "S";
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
	if (!task.uc_stack.ss_sp) {
		perror("malloc");
		exit(1);
	}
	task.uc_link = &terhd_ctx;

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
	} else {
		return -1;
	}
	PCB *tmp = create_pcb(t_n, t_q, task);
	gettimeofday(&(tmp->t_in), NULL);
	ready_queue->enq(ready_queue, create_node(tmp));
	return tmp->pid;
}

void sched_remove(const int pid)
{
	if (ready_queue == NULL || terminated_queue == NULL || waiting_queue == NULL) {
		return;
	}
	if (now_pcb->pid == pid) {
		FREE(now_pcb);
		return;
	}
	int num = waiting_queue->size(waiting_queue);
	node *tmp_node = NULL;
	while (num--) {
		tmp_node = waiting_queue->deq(waiting_queue);
		if (tmp_node->pcb->pid == pid) {
			FREE(tmp_node->pcb);
			FREE(tmp_node);
			return;
		}
		waiting_queue->enq(waiting_queue, create_node(tmp_node->pcb));
	}
	num = ready_queue->size(ready_queue);
	while (num--) {
		tmp_node = ready_queue->deq(ready_queue);
		if (tmp_node->pcb->pid == pid) {
			FREE(tmp_node->pcb);
			FREE(tmp_node);
			return;
		}
		ready_queue->enq(ready_queue, create_node(tmp_node->pcb));
	}
	num = terminated_queue->size(terminated_queue);
	while (num--) {
		tmp_node = terminated_queue->deq(terminated_queue);
		if (tmp_node->pcb->pid == pid) {
			FREE(tmp_node->pcb);
			FREE(tmp_node);
			return;
		}
		terminated_queue->enq(terminated_queue, create_node(tmp_node->pcb));
	}
	printf("pid not found\n");
}

void sched_ps()
{
	if (ready_queue == NULL || terminated_queue == NULL || waiting_queue == NULL) {
		return;
	}
	if (is_having_now && now_pcb != NULL) {
		printf("%*d  %s\t%s\t%ld\n",
		       4,
		       now_pcb->pid,
		       now_pcb->name,
		       get_pcb_state(now_pcb->state),
		       now_pcb->q_t);
	}
	// printf("ready:\n");
	node *curr = ready_queue->head;
	while (curr != NULL) {
		printf("%*d  %s\t%s\t%ld\n",
		       4,
		       curr->pcb->pid,
		       curr->pcb->name,
		       get_pcb_state(curr->pcb->state),
		       curr->pcb->q_t);
		curr = curr->next;
	}
	// printf("waiting:\n");
	curr = waiting_queue->head;
	while (curr != NULL) {
		// printf("%d\t%s\t%s\t%ld\t%ld\n",
		printf("%*d  %s\t%s\t%ld\t\n",
		       4,
		       curr->pcb->pid,
		       curr->pcb->name,
		       get_pcb_state(curr->pcb->state),
		       curr->pcb->q_t //,
		       // curr->pcb->s_t
		      );
		curr = curr->next;
	}
	// printf("terminated:\n");
	curr = terminated_queue->head;
	while (curr != NULL) {
		printf("%*d  %s\t%s\t%ld\n",
		       4,
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
		printf("\n");
		store_ready_task_time();
		swapcontext(&now_ctx, &shell_ctx);
	} else if (signum == SIGALRM) {
		is_simulating = true;
		if (!is_ctrlz && now_pcb != NULL)
			swapcontext(&now_pcb->ctx, &sched_ctx);
	}
}

void scheduler()
{
	if (is_ctrlz && now_pcb != NULL) {
		is_ctrlz = false;
		ucontext_t gg_ctx;
		swapcontext(&gg_ctx, &now_ctx);
	} else if (is_terminated) {
		is_terminated = false;
	} else if (is_simulating && now_pcb != NULL) { // && !is_terminated) {
		is_simulating = false;
		now_pcb->state = TASK_READY;
		gettimeofday(&(now_pcb->t_in), NULL);
		ready_queue->enq(ready_queue, create_node(now_pcb));
	}

	while (ready_queue != NULL && ready_queue->size(ready_queue) != 0) {
		now_pcb = ready_queue->deq(ready_queue)->pcb;
		/*Update queueing time*/
		// struct timeval t_out;
		// gettimeofday(&t_out, NULL);
		// long past_time = ((t_out.tv_usec - now_pcb->t_in.tv_usec) / 1000 + 1000) % 1000;
		// printf("%ld-%ld=%ld\n",
		// t_out.tv_usec / 1000,
		// now_pcb->t_in.tv_usec / 1000,
		// past_time);
		// now_pcb->q_t += past_time;
		long past_time = update_queueing_time(&now_pcb);
		is_having_now = true;
		now_pcb->state = TASK_RUNNING;
		/*Update waiting tasks' suspend time*/
		update_waiting_queue(past_time);
		/*timer*/
		memset(&it, 0, sizeof it);
		// it.it_value.tv_sec = ((now_pcb->t_q == 'S') ? 1 : 2); // TODO ms
		// it.it_interval.tv_sec = ((now_pcb->t_q == 'S') ? 1 : 2); // TODO ms
		it.it_value.tv_usec = ((now_pcb->t_q == 'S') ? 10000 : 20000);
		it.it_interval.tv_usec = ((now_pcb->t_q == 'S') ? 10000 : 20000);
		if (setitimer(ITIMER_REAL, &it, 0)) {
			perror("setitimer");
			exit(1);
		}
		ucontext_t gg_ctx;
		swapcontext(&gg_ctx, &now_pcb->ctx);
	}
	/*When ready_queue empty, keep update suspend time with every tasks*/
	while (waiting_queue != NULL && waiting_queue->size(waiting_queue) != 0) {
		update_waiting_queue(10);
		ucontext_t gg_ctx;
		swapcontext(&gg_ctx, &sched_ctx);
		/*timer*/
		memset(&it, 0, sizeof it);
		// it.it_value.tv_sec = 1; // TODO ms
		// it.it_interval.tv_sec = 1; // TODO ms
		it.it_value.tv_usec = 10000;
		it.it_interval.tv_usec = 10000;
		if (setitimer(ITIMER_REAL, &it, 0)) {
			perror("setitimer");
			exit(1);
		}
	}
	/*All tasks are terminated*/
	ucontext_t gg_ctx;
	swapcontext(&gg_ctx, &shell_ctx);
}

void update_waiting_queue(const int past_time)
{
	int waiting_num = waiting_queue->size(waiting_queue);
	while (waiting_num--) {
		PCB *tmp_pcb = waiting_queue->deq(waiting_queue)->pcb;
		tmp_pcb->s_t -= past_time;
		if (tmp_pcb->s_t <= 0) {
			tmp_pcb->state = TASK_READY;
			gettimeofday(&(tmp_pcb->t_in), NULL);
			ready_queue->enq(ready_queue, create_node(tmp_pcb));
			continue;
		}
		waiting_queue->enq(waiting_queue, create_node(tmp_pcb));
	}
}

long update_queueing_time(PCB **pcb)
{
	struct timeval t_out;
	gettimeofday(&t_out, NULL);
	long past_time = ((t_out.tv_usec - (*pcb)->t_in.tv_usec) / 1000 + 1000) % 1000;
	(*pcb)->q_t += past_time;
	return past_time;
}

void store_ready_task_time()
{
	int ready_num = ready_queue->size(ready_queue);
	while (ready_num--) {
		PCB *tmp_pcb = ready_queue->deq(ready_queue)->pcb;
		// struct timeval t_out;
		// gettimeofday(&t_out, NULL);
		// long past_time = ((t_out.tv_usec - tmp_pcb->t_in.tv_usec) / 1000 + 1000) % 1000;
		// now_pcb->q_t += past_time;
		update_queueing_time(&tmp_pcb);
		ready_queue->enq(ready_queue, create_node(tmp_pcb));
	}
}

void reset_ready_task_time()
{
	int ready_num = ready_queue->size(ready_queue);
	while (ready_num--) {
		PCB *tmp_pcb = ready_queue->deq(ready_queue)->pcb;
		gettimeofday(&tmp_pcb->t_in, NULL);
		ready_queue->enq(ready_queue, create_node(tmp_pcb));
	}
}

void terminated_handler()
{
	is_simulating = false;
	now_pcb->state = TASK_TERMINATED;
	// struct timeval t_out;
	// gettimeofday(&t_out, NULL);
	// long past_time = ((t_out.tv_usec - now_pcb->t_in.tv_usec) / 1000 + 1000) % 1000;
	// now_pcb->q_t += past_time;
	update_queueing_time(&now_pcb);
	terminated_queue->enq(terminated_queue, create_node(now_pcb));
	now_pcb = NULL;
	is_terminated = true;
	ucontext_t gg_ctx;
	swapcontext(&gg_ctx, &sched_ctx);
}

void hw_suspend(int msec_10)
{
	now_pcb->s_t = msec_10 * 10;
	now_pcb->state = TASK_WAITING;
	// struct timeval t_out;
	// gettimeofday(&t_out, NULL);
	// long past_time = ((t_out.tv_usec - now_pcb->t_in.tv_usec) / 1000 + 1000) % 1000;
	// now_pcb->q_t += past_time;
	update_queueing_time(&now_pcb);
	swapcontext(&now_pcb->ctx, &savsu_ctx);
	return;
}

void save_suspend()
{
	waiting_queue->enq(waiting_queue, create_node(now_pcb));
	now_pcb = NULL;
	ucontext_t gg_ctx;
	swapcontext(&gg_ctx, &sched_ctx);
}

void hw_wakeup_pid(int pid)
{
	int waiting_num = waiting_queue->size(waiting_queue);
	while (waiting_num--) {
		PCB *tmp_pcb = waiting_queue->deq(waiting_queue)->pcb;
		if (tmp_pcb->pid == pid) {
			gettimeofday(&(tmp_pcb->t_in), NULL);
			ready_queue->enq(ready_queue, create_node(tmp_pcb));
			return;
		}
		waiting_queue->enq(waiting_queue, create_node(tmp_pcb));
	}
	return;
}

int hw_wakeup_taskname(char *task_name)
{
	int count = 0;
	int waiting_num = waiting_queue->size(waiting_queue);
	while (waiting_num--) {
		PCB *tmp_pcb = waiting_queue->deq(waiting_queue)->pcb;
		if (strcmp(tmp_pcb->name, task_name) == 0) {
			count++;
			gettimeofday(&(tmp_pcb->t_in), NULL);
			ready_queue->enq(ready_queue, create_node(tmp_pcb));
			continue;
		}
		waiting_queue->enq(waiting_queue, create_node(tmp_pcb));
	}
	return count;
}

int hw_task_create(char *task_name)
{
	return sched_add(task_name, 'S');
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
		printf("0\n");
		return NULL;
	}
	node *tmp = self->tail;
	if (self->size(self) != 1) {
		self->tail = self->tail->prev;
		self->tail->next = NULL;
	} else {
		self->tail = NULL;
		self->head = NULL;
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
	if (self == NULL) {
		return false;
	}
	printf("head->\n");
	node *curr = self->head;
	int count = 0;
	while (curr != NULL) {
		printf("%d: %d\t%s\t%s\t%ld\n",
		       count++,
		       curr->pcb->pid,
		       curr->pcb->name,
		       get_pcb_state(curr->pcb->state),
		       curr->pcb->q_t);
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
	// tmp->t_l = (t_q == 'S') ? 10 : 20;
	tmp->state = TASK_READY;
	tmp->q_t = 0;
	tmp->ctx = ctx;
	return tmp;
}
