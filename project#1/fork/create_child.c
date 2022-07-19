#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t pid;

    /* 새로운 자식 프로세스 fork */
    /* pid_t fork(void) 해당 프로세스가 부모인지, 자식인지는 fork 의 return값으로 알수 있음 */
    pid = fork();

    if(pid < 0){ /* 에러 발생 예외 처리 */
        fprintf(stderr, "Fork failed");
        return 1;     
    }

    else if(pid == 0){ /*자식 프로세스 */
        printf("====================================== \n");
        printf("CHILD: ls command \n");
        execlp("/bin/ls", "ls", NULL); /* exec 호출로  /bin/ls 의해서 덮어 씌워짐 */
        printf("execlp call was unsuccessful \n"); /* 실행 되지 않는 이유는 새로운 프로세스로 대체 되어서 printf 명령으로 들어오지 않음 */
    }
    else{ /* 부모 프로세스 */
        wait(NULL); /* 자식 프로세스가 실행 종료 될때까지 기다림 */
        /* waitpid(pid,NULL,0); */
        printf("====================================== \n");
        printf("PARENT: child Complete \n");
        printf("====================================== \n");
    }

    return 0;
}
