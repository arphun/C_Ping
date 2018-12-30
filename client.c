#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
//#define debug 0
#define MULTIHOST 50
typedef struct serverinfo{
    int number;
    int timeout;
    char host[30];
    char port[16];
}Servinfo;
void thr_ping(void *data){
    Servinfo *s = (Servinfo*)data;
    int i ;
    int number = s->number;
    int timeout = s->timeout;
    char host[30];
    char port[16];
    strcpy(host, s->host);
    strcpy(port, s->port);
    int try = 0;
    int status, fd;
    char ip[INET6_ADDRSTRLEN];
    struct addrinfo hints;
    while(1){
        if(number > 0){
            if(try == number)
                pthread_exit((void *)0);
            
        }

        struct addrinfo *res;
        // getaddrinfo
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if((status = getaddrinfo(host, port, &hints, &res))!=0){
            printf("getaddrinfo failed\n");
            printf("errno=%d\n", errno);
            pthread_exit((void *)0);
        }
        void *addr;
        struct sockaddr_in *addrinfo=(struct sockaddr_in*)res->ai_addr;
        addr = &(addrinfo->sin_addr);
        inet_ntop(res->ai_family, addr, ip, sizeof(ip));
        if((status = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1){
            printf("socket establish error\n");
            pthread_exit((void *)0);

        }else{
            fd = status;
#ifdef debug
            printf("socket success, fd:%d\n",fd);
#endif
        }
        char msg[]= "test\n";
        char recv_msg[200];
        struct timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000)* 1000;
        fd_set readfd;
        FD_ZERO(&readfd);
        FD_SET(fd, &readfd);
        struct timeval time1,time2;
        if((connect(fd, res->ai_addr, res->ai_addrlen)) == -1){
            printf("connection error timeout when connect to [%s]\n", ip);     
            pthread_exit((void *)0);
        }else{
#ifdef debug
            printf("connect to server\n");
#endif    
        }
        gettimeofday(&time1, NULL);
        int byte = send(fd, msg, strlen(msg), 0);
        //select
        FD_ZERO(&readfd);
        FD_SET(fd,&readfd);
        select(fd+1, &readfd, NULL, NULL, &tv);
  //      printf("errno = %d\n", errno);   
        if(FD_ISSET(fd, &readfd)){
            gettimeofday(&time2, NULL);
            int RTT = (time2.tv_sec - time1.tv_sec)* 1000+(time2.tv_usec - time1.tv_usec)/1000;
            if( RTT > timeout ){
                printf("recv,timeout when connect to [%s]\n", ip); 
                pthread_exit((void *)0);
            }else{
                printf("recv from %s:%s, RTT = %d msec\n", ip, port, RTT);    
            }
        }else{
            printf("timeout when connect to [%s]\n", ip);
            pthread_exit((void *)0);
        }
        // print time
        close(fd);
        freeaddrinfo(res);
        try++;
        sleep(1);
    }
}
int main(int argc, char **argv){
    int i;
    int packet_number = 0,timeout = 1000;
    char host_port[MULTIHOST][46], host[MULTIHOST][30], port[MULTIHOST][16];
    int host_number = 0;
    // deal with the input format 
    for(i = 1; i<argc; i++){
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'n':
                    i++;
                    packet_number=atoi(argv[i]);
                    if(packet_number < 0){
                        printf("error: -n number, the number has to be greater than 0\n");
                    }
                    break;
                case 't':                    i++;
                    timeout=atoi(argv[i]);
                    if(timeout <= 0){
                        printf("error: -t timeout, the timeout has to be greater than 0\n");
                        exit(1);
                    }
                    break;
                default:
                    printf("command format error\n ./client [-n number] [-t timeout] host_1:post_1 host_2:post_2... \n");
                    exit(1);
                    break;
            
            }
            
        }else{
            sscanf(argv[i],"%[^:]:%s",host[host_number],port[host_number]);
                 host_number++;
        }
    }
#ifdef debug
    //printf("timeout is %d, packet_number is %d host_number = %d\n",timeout,packet_number,host_number);
    for(i = 0 ; i < host_number;i++){
        printf("host = %s, port= %s\n",host[i],port[i]);   
    }
#endif
    // socket and connect
    Servinfo servinfo[host_number];
    for(i = 0; i < host_number;i++){
        strcpy(servinfo[i].host, host[i]);
        strcpy(servinfo[i].port, port[i]);
        servinfo[i].number = packet_number;
        servinfo[i].timeout = timeout;
    }
    pthread_t tid[host_number];
    for(i = 0; i < host_number; i++){
        // pass the fd, n, t, ip, res to the thread to connect and ping  
        if((pthread_create(&tid[i], NULL, thr_ping, (void*)&servinfo[i]))!=0){
            printf("thread creation failed\n");
        }
    }
    void *tmp;
    for(i = 0 ; i < host_number; i++){
        pthread_join(tid[i], &tmp);
    }
    return 0;
}
