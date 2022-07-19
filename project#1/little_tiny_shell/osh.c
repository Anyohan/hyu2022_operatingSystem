#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <unistd.h>

typedef int bool;

#define MAX_LINE 80 /* the maximum length command */
#define true 1
#define false 0
#define READ_END 0
#define WRITE_END 1

bool is_right_operator;
bool is_left_operator;
bool ampersand_operator;
bool is_pipe_operator;



int main(void){
    char *args[MAX_LINE/2+1] = {NULL,}; /* command line argument */
    char *p_args[MAX_LINE/2+1] = {NULL,};
    int should_run = 1; /* flag to determine when to exit program */
    char input[MAX_LINE];
    pid_t pid;
    pid_t pid2;
    int i; 
    int backgournd = 0; /* 백그라운드 확인 변수 */
    int p_fd[2]; /* 파이프 변수 */

    while(should_run){

        int fd;
        is_right_operator = false;
        is_left_operator = false;
        ampersand_operator = false;
        is_pipe_operator = false;

        /* 새로운 명령어를 칠때마다 명령어 배열을 초기화 */
        memset(args,0,(MAX_LINE/2+1)*sizeof(char*));

        printf("osh> ");
        fflush(stdout); 
        /* reading user input */

        fgets(input,MAX_LINE,stdin);

        /* 입력 커맨드가 아무것도 없을때 */
        if(input[0] == '\n'){
            continue;
        }

        input[strlen(input)-1] = '\0';  /* 해당 input 배열의 종료지점 지정 */

        char* file_name = NULL;
        i = 0;

        /* 명령어를 받아서 띄어쓰기 기준으로 토크나이즈  */
        char* tmp = strtok(input," ");

        

        /* 명령어 or 커맨드가 끝나지 않을때까지 */
        while(tmp != NULL){
            /* <,>,&  */
            if(strchr(tmp,'>')){
                is_right_operator = true;
                file_name = strtok(NULL," ");
            }
            else if(strchr(tmp,'<')){
                is_left_operator = true;
                file_name = strtok(NULL," ");
            }

            if(strchr(tmp,'&')){
                ampersand_operator = true;
            }

            /* 파이프 오퍼레이터 뒤에 명령문 검사 */
            if(strchr(tmp,'|')){
                i = 0;
                is_pipe_operator = true;
                while(tmp != NULL){
                    if(strchr(tmp,'&')){
                        ampersand_operator = true;
                    }
                }
                if(!(ampersand_operator)){
                    p_args[i++] = tmp;
                }
            }

            /* 오퍼레이터가 없을때만 명령어 배열에 명령어 추가 */
            if(!(is_left_operator || is_right_operator || ampersand_operator)){
                args[i++] = tmp;
            }
            tmp = strtok(NULL," ");
        }

        /* exit 명령어시 쉘 종료 */
        if(strcmp(args[0],"exit") == 0){
            break;
            should_run = 0;
        }
        
        /* 백그라운드 프로세스 */
        if(ampersand_operator){
            backgournd = 1;
        }

        
        /* 자식생성 */
        pid = fork();

        /* 포크 에러 */
        if(pid < 0){
            printf("fork Error");
            exit(1);
        }

        /* 자식일떄 */
        if(pid == 0){
            /* 파일을 통한 입출력 처리 */
            if(is_right_operator){
                fd = open(file_name, O_CREAT | O_WRONLY,0755);
                dup2(fd,STDOUT_FILENO);
            }
            else if(is_left_operator){
                fd = open(file_name,O_RDONLY);
                dup2(fd,STDIN_FILENO);
            }

            /* 파이프를 판별하고 파이프를 열어 생성된 자식프로세스와 통신 */
            if(is_pipe_operator){
                pipe(p_fd);
                pid2 = fork();
                printf("%s",p_args[0]);
                if(pid2 < 0){
                    printf("fork Error \n");
                    return 1;
                }
                if(pid2 > 0){
                    close(p_fd[READ_END]);
                    dup2(p_fd[WRITE_END],STDOUT_FILENO);
                }
                else{
                    close(p_fd[WRITE_END]);
                    dup2(p_fd[READ_END],STDIN_FILENO);
                    execvp(p_args[0],p_args);
                }

            }
            
            printf("[pid] %d  \n ",pid);
            execvp(args[0],args);
            // exit(0);

        }

        /* 부모 프로세스 진행 */
        if(pid > 0){
            if(!backgournd){
                printf("[ppid] %d \n",pid);
                int retval = waitpid(pid,NULL,0);

                if(retval > 0){
                    printf("waiting for child, not a back ground process\n");
                }
                else{
                    printf("Unexpected error\n");
                    exit(0);
                }
                printf("child process complete\n");
            }
            else{
                printf("background process\n");
            }
        }
        backgournd = 0;
        close(fd);
        

    }
    return 0;
}