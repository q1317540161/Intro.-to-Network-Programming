#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h> 
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>

#define numCon 100
#define MAXLINE 50000
int     connfd[numCon];
char    buf[MAXLINE]={0};

int clistenfd=0;
void addConection(int* fd, char* ip, int port);
void disconnect(int i);

void sig_terminate(int signo){
    send(clistenfd, "/report\n", sizeof("/report\n"), MSG_NOSIGNAL);
    read(clistenfd, &buf, sizeof(buf));
    printf("\n%s", buf);
    exit(0);
}
void addConection(int* fd, char* ip, int port){
    struct sockaddr_in	servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(port);

    if ( (*fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror("socket error");
  
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
        perror("inet_pton error");

    if ( (connect(*fd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0)
        perror("connect error");
}
void disconnect(int i){
    close(connfd[i]);
    connfd[i]=-1;
}

int main(int argc, char **argv)
{   
    int cPort = atoi(argv[2]);
    int sPort = cPort+1;
    addConection(&clistenfd, argv[1], cPort);
  
    signal(SIGINT, sig_terminate);
    signal(SIGTERM, sig_terminate);

    for(int i=0; i<numCon; i++){
        addConection(&connfd[i], argv[1], sPort);
    }

    send(clistenfd, "/reset\n", sizeof("/reset\n"), MSG_NOSIGNAL);
    read(clistenfd, &buf, sizeof(buf));

    for(;;){
        for(int i=0; i<numCon; i++){
            if(send(connfd[i], &buf, sizeof(buf), MSG_NOSIGNAL)<0)
                if (errno == ECONNRESET || errno == EPIPE) disconnect(i);
        }
    }
    
  return 0;
}