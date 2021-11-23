#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>
#include <sys/types.h>  // socket, bind, accept, open
#include <sys/socket.h> // socket, bind, listen, accept
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <errno.h>
#include <pthread.h>

#define PORT    8070
#define MAXBUF  1024


int thread_num;
off_t fsize;
int th_completed = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct arg_struct {
    // int server_sockfd;
    // int client_sockfd;
    FILE* des_fd;
    // int client_len, read_len, file_read_len;
    intptr_t i;
    // struct sockaddr_in serveraddr, clientaddr;
};

void *thread_kernel(void *arguments) {

    struct arg_struct *arg = arguments;
    intptr_t t = arg->i;
    // int server_sockfd = arg->server_sockfd;
    // int client_sockfd = arg->client_sockfd;
    FILE* des_fd = arg->des_fd;
    // int client_len = arg->client_len;
    // int read_len = arg->read_len;
    // int file_read_len = arg->file_read_len;
    // struct sockaddr_in serveraddr = arg->serveraddr;
    // struct sockaddr_in clientaddr = arg->clientaddr;


    int server_sockfd;
    int client_sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    int client_len, read_len, file_read_len;
    // int des_fd;
    // intptr_t t = (intptr_t) arg->i;
    printf("i am thread %ld\n", t);
    // printf("des_fd: %d", des_fd);

    client_len = sizeof(clientaddr);


    char buf[MAXBUF];
    int check_bind;

    /* socket() */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd < 0) {
        printf("socket error : \n");
    }

    /* bind() */
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serveraddr.sin_port         = htons(PORT+t+1);
    // printf("%d port on!\n", PORT+t+1);

    if(bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) > 0) {
        printf("bind error : \n");
    }

    
    /*pthread_mutex_lock(&lock);
    th_completed++;
    if (th_completed == thread_num) {

        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&lock);*/

    /* listen */
    if(listen(server_sockfd, 5) != 0) {
        printf("listen error : \n");
    }

    client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);


    // if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&serveraddr, 
    //                    (socklen_t*)sizeof(serveraddr)))<0)
    // {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }
    
    printf("t -- %ld\n",  t);


    while (1) {
        
        pthread_mutex_lock(&lock);
        char buf[MAXBUF];
        memset(buf, 0x00, MAXBUF);
        // printf("here\n");
        file_read_len = read(client_sockfd, buf, MAXBUF);
        printf("%d\n", file_read_len);
        // printf("read len: %d", file_read_len);
        write(des_fd, buf, file_read_len);
        // fwrite(buf, 1, file_read_len, "video.mp4");
        pthread_mutex_unlock(&lock);
        // break;
        if(file_read_len == EOF | file_read_len == 0) {
            printf("finish file\n");
            break;
        }
    }
    return 0;
}


int main() {
    
    int server_sockfd;
    int client_sockfd;
    FILE* des_fd;
    int opt = 1;
    struct sockaddr_in serveraddr, clientaddr;
    int client_len, read_len, file_read_len;    // length


    char buf[MAXBUF];
    int check_bind;
    client_len = sizeof(clientaddr);

    // Creating socket file descriptor
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd <= 0) {
        perror("socket error : ");
        exit(EXIT_FAILURE);
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serveraddr.sin_port         = htons(PORT);

    /* bind() */
    if (bind(server_sockfd, (struct sockaddr *)&serveraddr, 
                                 sizeof(serveraddr))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* listen */
    if(listen(server_sockfd, 5) < 0) {
        perror("listen error : ");
        exit(EXIT_FAILURE);
    }

    // pthread_t thread_id[thread_num];

    // for (intptr_t i = 0; i < thread_num; ++i) {
    //     struct arg_struct args;
    //     args.server_sockfd = server_sockfd;
    //     args.client_sockfd = client_sockfd;
    //     args.des_fd = des_fd;
    //     args.client_len = client_len;
    //     args.read_len = read_len;
    //     args.file_read_len = file_read_len;
    //     args.i = i;
    //     args.serveraddr = serveraddr;
    //     args.clientaddr = clientaddr;

    //     pthread_create(&thread_id[i], NULL, thread_kernel, (void *)&args);
    // }

    // for (int i = 0; i < thread_num; ++i)
    //     pthread_join(thread_id[i], NULL);


    // client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
    // printf("New Client Connect : %s\n", inet_ntoa(clientaddr.sin_addr));
    // read_len = read(client_sockfd, buf, MAXBUF);
    // printf("%d\n", read_len);

    while(1) {
        char file_name[MAXBUF]; // local val
        memset(buf, 0x00, MAXBUF);

        /* accept() */
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
        printf("New Client Connect : %s\n", inet_ntoa(clientaddr.sin_addr));

        /* file name */
        read_len = read(client_sockfd, buf, MAXBUF);
        
        if(read_len > 0) {
            strncpy(file_name, buf, 50);
            read(client_sockfd, &thread_num, sizeof(thread_num));
            read(client_sockfd, &fsize, sizeof(fsize));
            printf("%s > %s %d %ld\n", inet_ntoa(clientaddr.sin_addr), file_name, thread_num, fsize);
        } else {
            close(client_sockfd);
            break;
        }

        /* create file */

        des_fd = fopen(file_name, "wb");
        if(!des_fd) {
            perror("file open error : ");
            break;
        }

        // printf("des_fd: %d", des_fd);
        pthread_t thread_id[thread_num];

        printf("threads: %d\n", thread_num);

        for (intptr_t i = 0; i < thread_num; ++i) {
            struct arg_struct args;
            // args.server_sockfd = server_sockfd;
            // args.client_sockfd = client_sockfd;
            args.des_fd = des_fd;
            // args.client_len = client_len;
            // args.read_len = read_len;
            // args.file_read_len = file_read_len;
            args.i = i;
            // args.serveraddr = serveraddr;
            // args.clientaddr = clientaddr;
           pthread_create(&thread_id[i], NULL, thread_kernel, (void *)&args);
           sleep(1);
        }


        //pthread_mutex_lock(&lock);
        //pthread_cond_wait(&cond, &lock);
        //pthread_mutex_unlock(&lock);

        //send(client_sockfd, &fsize, sizeof(fsize), 0);
        // printf("shit 2\n");
        //sleep(10); 

        //for (int i = 0; i < thread_num; ++i)
        //    pthread_join(thread_id[i], NULL);

        /* file save */
        /*while(1) {
            memset(buf, 0x00, MAXBUF);
            file_read_len = read(client_sockfd, buf, MAXBUF);
            write(des_fd, buf, file_read_len);
            if(file_read_len == EOF | file_read_len == 0) {
                printf("finish file\n");
                break;
            }
        }*/


        close(client_sockfd);
        close(des_fd);
    }
    close(server_sockfd);
    return 0;
}