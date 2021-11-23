#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>

#define PORT    5500
#define IP      "127.0.0.1"
#define MAXBUF  1024



struct sockaddr_in serv_addr;
int     s;
int         sourse_fd;
int         file_name_len, read_len;

off_t fsize;
int thread_num = 5;
pthread_mutex_t lock;

void *thread_kernel(void *arg) {
    printf("i am a thread %ld\n", (intptr_t)arg);
    while (1) {
        
        pthread_mutex_lock(&lock);
        char        buf[MAXBUF];

        memset(buf, 0x00, MAXBUF);
        read_len = read(sourse_fd, buf, MAXBUF);
        send(s, buf, read_len, 0);
        pthread_mutex_unlock(&lock);

        if(read_len == 0) {
            break;
        }
    }
    return 0;
}

int main() {

    char        buf[MAXBUF];
    
    /* socket() */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if(connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect : ");
        printf("fail to connect.\n");
        close(s);
        return 1;
    }

    memset(buf, 0x00, MAXBUF);
    printf("write file name to send to the server:  ");
    scanf("%s", buf);

    printf(" > %s %d %d\n", buf);
    file_name_len = strlen(buf);

    send(s, buf, file_name_len, 0);
    sourse_fd = open(buf, O_RDONLY);

    fsize = lseek(sourse_fd, 0, SEEK_END);
    printf("bytes: %ld\n", fsize);
    lseek(sourse_fd, 0, SEEK_END);

    if(!sourse_fd) {
        perror("Error : ");
        return 1;
    }


    pthread_t thread_id[thread_num]; 

    for (intptr_t i = 0; i < thread_num; ++i) {
        pthread_create(&thread_id[i], NULL, thread_kernel, (void *)i);
    }

    for (int i = 0; i < thread_num; ++i)
        pthread_join(thread_id[i], NULL);

/*
    while(1) {
        memset(buf, 0x00, MAXBUF);
        read_len = read(sourse_fd, buf, MAXBUF);
        send(s, buf, read_len, 0);
        if(read_len == 0) {
            break;
        }

    }*/

    return 0;
}