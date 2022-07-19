#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

int main()
{
	int	pid;
	int	status;
	int	retval;

	pid = fork();
	if (pid < 0) {
		perror("FORK ERROR:");
		exit(0);
	}
	if (pid == 0) {
		int	i;
		for (i = 0; i < 10; i++) {
			printf("Child : %d\n", i);
			sleep(3);
		}
		exit(3); /* 3뱉고 exit */
	} else {
		printf("Waiting for Child process (pid:%d)\n", pid);
		retval = waitpid(pid, &status, 0);
		printf("Child exits (status:%d)\n", status);

		if (retval > 0) {
			if (WIFEXITED(status)) {
				printf("Child exited by process completeion : %d\n", WEXITSTATUS(status));
			}
			if (WIFSIGNALED(status)) {
				printf("Child exited by signal : %d\n", WTERMSIG(status));
			}
		} else {
			printf("Unexpected error.\n");
			exit(0);
		}
	}
	return 0;
}
