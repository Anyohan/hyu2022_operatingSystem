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

bool is_right_operator;
bool is_left_operator;
bool ampersand_operator;




int main(void){
    char *args[MAX_LINE/2+1] = {NULL,}; /* command line argument */
    int should_run = 1; /* flag to determine when to exit program */
    char input[MAX_LINE];
    pid_t pid;


    while(should_run){

        int fd;
        is_right_operator = false;
        is_left_operator = false;
        ampersand_operator = false;
        pid = 0;

        printf("osh> ");
        fflush(stdout); 
        /* reading user input */

        fgets(input,MAX_LINE,stdin);


        input[strlen(input)-1] = '\0';  /* 해당 input 배열의 종료지점 지정 */

        char* file_name = NULL;
        int i = 0;

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

            /* 오퍼레이터가 없을때만 명령어 배열에 명령어 추가 */
            if(!(is_left_operator || is_right_operator || ampersand_operator )){
                args[i++] = tmp;
            }
            tmp = strtok(NULL," ");
        }
        
        /* 파일을 통한 입출력 처리 */
        if(is_right_operator){
            fd = open(file_name, O_CREAT | O_WRONLY,0755);
            dup2(fd,STDOUT_FILENO);
        }
        else if(is_left_operator){
            fd = open(file_name,O_RDONLY);
            dup2(fd,STDIN_FILENO);
        }

        
        /* background process */
        if(ampersand_operator){
            pid = fork();
            /* 자식 프로세스가 백그라운드에서 실행 */
            if(pid != 0){
                printf("parent %d \n",pid);
            }
        }

        if(pid < 0){
            printf("fork Error! \n");
            exit(1);
        }

        /* &가 없다면 pid 를 0 으로 초기화했기때문에 자식이 아닌 부모가 해당 작업 수행 */
        else if(pid == 0){
            //printf("process start \n");
            execvp(args[0],args);
        }


        else{
            wait(NULL);
            printf(" %d process done \n ",pid);
        }
        close(fd);
        

    }
    return 0;
}

