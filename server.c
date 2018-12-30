#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#define debug 1
#define BACKLOG 20
int main(int argc, char **argv){
    int socket_fd;
    if(argc != 2){
        printf("Command line error, the format shoube be /server portnumber");
        exit(1);
    }else{
#ifdef debug
        printf("port is %s\n",argv[1]);
#endif
    }
	printf("err : %d\n", errno);
    
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; //for TCP socket
    hints.ai_flags = AI_PASSIVE;
	printf("err : %d\n", errno);
    if((status = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0){
        printf("getaddrinfo error\n");
        exit(1);
    }else{
#ifdef debug
    printf("%d %d %d\n", AF_UNSPEC, AF_INET, AF_INET6);
    printf("%d\n", servinfo->ai_family);
    printf("%d\n", servinfo->ai_socktype==SOCK_STREAM);
    printf("%d\n", servinfo->ai_protocol==getprotobyname("tcp")->p_proto);
	printf("err : %d\n", errno);
#endif
    }
    if((status = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol ))==-1){
        printf("socket establish error\n");
    }else{
        socket_fd = status;
#ifdef debug 
        printf("socket success , at %d\n",socket_fd);
#endif
    }
    void *addr;
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)servinfo->ai_addr;
    addr = &(ipv6->sin6_addr);
    inet_ntop(servinfo->ai_family, addr, ipstr, sizeof(ipstr));
    printf("ip:[%s]\n",ipstr);

    /*
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons((uint16_t)atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    servinfo->ai_addr =(struct sockaddr *)&server_addr;
    servinfo->ai_addrlen = sizeof(server_addr);
    */
    printf("err : %d\n", errno);
    if((status = bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
        printf("bind error\n");
    }
    printf("err : %d\n", errno);
/*#ifdef debug
    void *addr;
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
    addr = &(ipv4->sin_addr);
    inet_ntop(servinfo->ai_family, addr, ipstr, sizeof(ipstr));
    printf("ip:%s\n",ipstr);
#endif
*/
    if((status = listen(socket_fd, BACKLOG))==-1){
            printf("listen error\n");
    }
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    memset(&client_addr, 0, sizeof(client_addr));
    char msg[1000]="test\n";
    char recv_msg[1000]={""};
    while(1){
	int client_fd;
        client_fd = accept(socket_fd,(struct sockaddr*)&client_addr, &client_addr_size);
        printf("recv from %s:%d\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        int number = recv(client_fd, recv_msg, sizeof(recv_msg), 0);
    	printf("err : %d\n", errno);
	printf("recv %d words, recv_message is [%c]\n", number, recv_msg[0]);
	send(client_fd, msg, strlen(msg), 0 );
	printf("close the client\n");
        close(client_fd);
    }

    return(0);
    
}
