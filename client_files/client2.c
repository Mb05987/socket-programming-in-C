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

#define PORT    8070
#define IP      "127.0.0.1"
#define MAXBUF  1024




// int thread_num = 3;
pthread_mutex_t lock;

struct arg_struct {
    intptr_t i;
    int sourse_fd;
    off_t fsize;
    int file_name_len;
};


void *thread_kernel(void *arguments) {
    struct arg_struct *arg = arguments;
    intptr_t t = arg->i;
    int sourse_fd = arg->sourse_fd;
    off_t fsize = arg->fsize;
    int file_name_len = arg->file_name_len;


    printf("i am thread %ld\n", t);
    

    struct sockaddr_in serv_addr;
    int s;
    int read_len;
    char buf[MAXBUF];
    
    /* socket() */
    s = socket(AF_INET, SOCK_STREAM, 0);
    
    if(s < 0) {
        printf("socket error : \n");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT+t+1);
    
    // printf("%d port on!\n", PORT+t+1);
    
    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
    }
    
    if(connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("fail to connect.\n");
        close(s);
    }
    // else{
    //     printf("connected successfully!");
    // }
    // printf("here\n");
    
    while (1) {
        
        pthread_mutex_lock(&lock);
        char buf[MAXBUF];

        memset(buf, 0x00, MAXBUF);
        // printf("%d\n", sourse_fd);
        read_len = read(sourse_fd, buf, MAXBUF);
        // printf("%ld %d\n", t, read_len);
        send(s, buf, read_len, 0);
        pthread_mutex_unlock(&lock);

        if(read_len == 0) {
            break;
        }
    }
    printf("%ld done\n", t);
    return 0;
}

int main() {
    struct sockaddr_in serv_addr;
    int   s;
    int   sourse_fd;
    int   file_name_len, read_len;
    int   thread_num;
    off_t fsize;
    char        buf[MAXBUF];
    
    /* socket() */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0) {
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, IP, &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if(connect(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("fail to connect.\n");
        // close(s);
        return -1;
    }
    else{
        printf("connected!\n");
    }

    memset(buf, 0x00, MAXBUF);
    printf("write file name to send to the server:  ");
    scanf("%s", buf);
    printf("write number of threads:  ");
    scanf("%d", &thread_num);
    
    sourse_fd = open(buf, O_RDONLY);
    fsize = lseek(sourse_fd, 0, SEEK_END);
    printf("bytes: %ld\n", fsize);
    lseek(sourse_fd, 0, SEEK_SET);


    printf(" > %s %d %ld\n", buf, thread_num, fsize);
    file_name_len = strlen(buf);

    send(s, buf, MAXBUF, 0);
    send(s, &thread_num, sizeof(thread_num), 0);
    send(s, &fsize, sizeof(fsize), 0);

    if(!sourse_fd) {
        perror("Error : ");
        return 1;
    }


    pthread_t thread_id[thread_num];

    read(s, &fsize, sizeof(fsize));
    sleep(1);
    for (intptr_t i = 0; i < thread_num; ++i) {
        struct arg_struct args;
        args.sourse_fd = sourse_fd;
        args.file_name_len = file_name_len;
        args.fsize = fsize;
        args.i = i;

        pthread_create(&thread_id[i], NULL, thread_kernel, (void *)&args);
        sleep(1);
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