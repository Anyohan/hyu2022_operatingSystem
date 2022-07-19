#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define HEADER_FMT "HTTP/1.1 %d %s \nContent_Length: %ld\nContent_Type: %s\n\n"

#define NOT_FOUND_CONTENT "<h1>404 Not Found</h1>\n"
#define SERVER_ERROR_CONTENT "<h1>500 Internet Server Error</h1>\n"

void fill_header(char *header, int status, long len, char *type);
void content_type(char *ct_type, char *url);
void handler_404(int newsockfd);
void handler_500(int newsockfd);
void http_handler(int newsockfd);
void error_check(char* message);


int bind_servsock(int sockfd, int port){
    // 소켓 주소를 기반으로 바인드
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    return bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
}

int main(int argc, char *argv[]){
    int port;
    int sockfd,newsockfd;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len;
    socklen_t serv_addr_len;

    int pid;

    if(argc != 2){
        printf("Usage : %s <port> \n",argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    printf("[INFO] server will listen to port: %d \n",port);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);

    // bind 및 accept 예외처리
    if(sockfd < 0){
        error_check("failed to create socket");
    }
    if(bind_servsock(sockfd,port) == -1){
        error_check("falied to bind");
    }
    if(listen(sockfd, 20) == -1){
        error_check("failed to listen");
    }

    // 무한루프로 구현한 통신
    while(1){
        printf("[INFO] waiting.....\n");
        newsockfd = accept(sockfd, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
        if(newsockfd < 0){
            error_check("failed to accept");
            continue;
        }
        pid = fork();
        if(pid == 0){http_handler(newsockfd);  exit(0);}
        if(pid != 0){close(newsockfd);}
        if(pid<0){printf("fork failed");}

        // pthread_create(&tid,NULL,http_handler,newsockfd);
        // pthread_detach(tid);
    }
    close(sockfd);
    return 0;
}

void fill_header(char *header, int status, long len, char *type){
    char status_text[40];
    switch(status) {
        case 200:
            strcpy(status_text, "OK"); break;
        case 404:
            strcpy(status_text, "NOT Found"); break;
        case 500:
        default:
            strcpy(status_text, "Internet Server Error"); break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type);
}

/* 클라이언트한테 받아온 content 타입 검사 및 문자열 파싱 */
void content_type(char *ct_type, char *url){
    char *tmp = strrchr(url, '.');
    int count = 1;
    if(!strcmp(tmp, ".html")){strcpy(ct_type, "text/html");}
    else if(!strcmp(tmp,  ".jpeg") || ! strcmp(tmp, ".jpeg")){strcpy(ct_type,"image/jpeg");}
    else if(!strcmp(tmp, ".png")){strcpy(ct_type, "image/png");}
    else if(!strcmp(tmp, ".ico")){
        if(count == 1){
            count = 0;
        }
    }
}

/* 404 handler */
void handler_404(int newsockfd){
    char header[BUF_SIZE];
    fill_header(header, 404, sizeof(NOT_FOUND_CONTENT),"text/html");
    write(newsockfd,header,strlen(header));
    write(newsockfd, NOT_FOUND_CONTENT, sizeof(NOT_FOUND_CONTENT));
}

/* 500 handler */
void handler_500(int newsockfd){
    char header[BUF_SIZE];
    fill_header(header, 500, sizeof(SERVER_ERROR_CONTENT),"text/html");
    write(newsockfd,header,strlen(header));
    write(newsockfd, SERVER_ERROR_CONTENT, sizeof(SERVER_ERROR_CONTENT));
}

/* req,res 송수신 */
void http_handler(int newsockfd){
    char header[BUF_SIZE]; 
    char buf[BUF_SIZE]; //http 메세지 버퍼
    if(read(newsockfd, buf, BUF_SIZE) < 0){
        error_check("failed to read request. \n");
        handler_500(newsockfd);
        return;
    }
    printf("%s",buf);
    char *url = strtok(buf, " ");
    url = strtok(NULL," ");
    // url == "/"
    if(url == NULL){
        error_check("failed to identify method, url. \n");
        handler_500(newsockfd);
        return;
    }

    printf("[INFO] Handling Request:url=%s\n",url);

    // 현재 하위 디렉토리의 경로를 local_url 에 복사
    char *local_url = getenv("PWD");
    struct stat st;
    
    if(strcmp(url, "/") == 0 || strcmp(url,"/favicon.ico") == 0){
        strcpy(url,"/index.html");
    }
    strcat(local_url,url);

    

    if(stat(local_url, &st) < 0){
        error_check("[WARN] No file found matching url.\n");
        handler_404(newsockfd);
        return;
    }
    
    int fd = open(local_url,O_RDONLY);

    int ct_len = st.st_size;
    char ct_type[40];
    content_type(ct_type, url);
    fill_header(header, 200, ct_len, ct_type);
    write(newsockfd, header, strlen(header));

    int cnt;
    while((cnt = read(fd, buf, BUF_SIZE)) > 0){
        write(newsockfd, buf, cnt);
    }
}

void error_check(char* msg){
    perror(msg);
    exit(1);
}