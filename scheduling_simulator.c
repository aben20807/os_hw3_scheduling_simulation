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
}

void s_ps()
{
	printf("ps\n");
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
	return 0; // the pid of created task name
}
