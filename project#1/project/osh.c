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

    int fd;
    int should_run = 1; /* 프로그램 실행과 종료를 위한 변수 */
    char* input;
    
    /* 자식 프로세스의 식별을 위한 pid */
    pid_t pid;
    pid_t pid2;

    int i; /* 배열의 메모리 할당을 위해 전역변수로 초기화 */
    int j;
    int backgournd = 0; /* 백그라운드 확인 변수 */

    while(should_run){
        is_right_operator = false;
        is_left_operator = false;
        ampersand_operator = false;
        is_pipe_operator = false;

        char *args[MAX_LINE/2+1] = {NULL,}; /* 사용자 명령어 저장 배열 */
        char *p_args[MAX_LINE/2+1] = {NULL,}; /* 두번쨰 자식 명령어 저장 배열 */

        input = (char*) malloc(MAX_LINE*sizeof(char));
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
            /* <,>,& ,| 검사 */
            if(strchr(tmp,'>')){
                is_right_operator = true;
                file_name = strtok(NULL," ");
            }
            else if(strchr(tmp,'<')){
                is_left_operator = true;
                file_name = strtok(NULL," ");
            }

            else if(strchr(tmp,'&')){
                ampersand_operator = true;
            }
            
            else if(strchr(tmp,'|')){
                i = 0;
                is_pipe_operator = true;
                //tmp = strtok(NULL," ");
            }
            
            if(is_pipe_operator){
                if(!strchr(tmp,'|')){
                    p_args[i++] = tmp;
                }
            }
            else if(!(is_left_operator || is_right_operator || ampersand_operator)){
                if(!strchr(tmp,'|')){
                    args[i++] = tmp;
                }
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
            if(is_pipe_operator){
                int p_fd[2];
                
                if (pipe(p_fd) == -1){
                    exit(1);
                }

                pid2 = fork();

                if(pid2<0){
                    exit(1);
                }

                if(pid2 == 0){
                    dup2(p_fd[READ_END],STDIN_FILENO);
                    close(p_fd[WRITE_END]);
                    execvp(p_args[0],p_args);
                }
                dup2(p_fd[WRITE_END],STDOUT_FILENO);
                close(p_fd[READ_END]);
            }
            execvp(args[0],args);
            

        }

        /* 부모 프로세스 진행 */
        if(pid > 0){
            if(!backgournd){
                //printf("[0] %d \n",pid);
                int retval = waitpid(pid,NULL,0);

                if(retval > 0){
                    //printf("waiting for child, not a back ground process\n");
                }
                else{
                    printf("Unexpected error\n");
                    exit(0);
                }
                //printf("child process complete\n");
            }
            else{
                waitpid(-1,NULL,WNOHANG);
                printf("background process\n");
            }
        }
        backgournd = 0;
        close(fd);
        free(input);
    }
    return 0;
}