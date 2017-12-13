#include "scheduling_simulator.h"

int main()
{
	while (1) {
		command_handler();
	}
	return 0;
}

void command_handler()
{
	printf("$ ");
	char command[20];
	fgets(command, 20, stdin);
	switch (command[0]) {
	case 'a':
		printf("a\n");
		break;
	case 'r':
		printf("r\n");
		break;
	case 's':
		printf("s\n");
		break;
	case 'p':
		printf("p\n");
		break;
	default:
		printf("ERROR COMMAND\n");
	}
}

void s_add(const char *t_n, const char t_q);

void s_remove(const int pid);

void s_start();

void s_ps();

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
	return 0; // the pid of created task name
}
