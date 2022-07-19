#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int value = 5;
#define	BUFFER_SIZE	1024
#define	READ_END	0
#define	WRITE_END	1

int main()
{
	char buf[BUFFER_SIZE];
	int fd[2];
	pid_t pid;

	pid = fork();

	if (pid == 0) {
		close(fd[READ_END]);
		sprintf(buf,"%d",value+15);
		write(fd[WRITE_END],buf,BUFFER_SIZE);
		close(fd[WRITE_END]);
		
		
	} else if (pid > 0) {
		close(fd[WRITE_END]);
		read(fd[READ_END],buf,BUFFER_SIZE);
		close(fd[READ_END]);
		printf("PARENT: value = %s\n", buf);
		exit(0);
		/* LINE A */
	}
	
	return 0;
}

/* 과연 부모한테 영향이 갈까? */
/* 바뀌지 않는다. */
/* | 는 어떤 의미일까 */
