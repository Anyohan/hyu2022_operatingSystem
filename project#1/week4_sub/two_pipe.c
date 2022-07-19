#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	BUFFER_SIZE	1024
#define	READ_END	0
#define	WRITE_END	1

int main(void)
{
	char buf[BUFFER_SIZE];
	int fdA[2];
    int fdB[2];
	pid_t pid;

	if (pipe(fdA) == -1 || pipe(fdB) == -1) {
		printf("PIPE ERROR\n");
		fprintf(stderr, "Pipe failed");
		return 1;
	}
	pid = fork();

	if (pid < 0) {
		printf("FORK ERROR\n");
		fprintf(stderr, "Fork failed");
		return 1;
	}
    if (pid > 0) {
            int countP = 0;
            close(fdA[READ_END]);
            close(fdB[WRITE_END]);
            while(1){
                sprintf(buf,"parent %d ",countP++);


                write(fdA[WRITE_END], buf, BUFFER_SIZE);

                memset(buf,0,sizeof(buf));
                
                read(fdB[READ_END],buf,BUFFER_SIZE);

                printf("parent got message:%s \n",buf);

                sleep(1);
            }

        } else {
            int countJ = 10000;
            close(fdA[WRITE_END]);
            close(fdB[READ_END]);

            while(1){
                sprintf(buf,"child %d ",countJ++);

                close(fdB[READ_END]);

                write(fdB[WRITE_END],buf,BUFFER_SIZE);

                memset(buf,0,sizeof(buf));

                read(fdA[READ_END], buf, BUFFER_SIZE);

                printf("child got message:%s \n",buf);


                sleep(1);
            }
        }
        exit(0);
        
    }
