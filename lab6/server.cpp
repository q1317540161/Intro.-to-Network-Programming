#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<time.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h> 
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<poll.h>
#include<thread>

using namespace std;

#define OPEN_MAX_C 5
#define OPEN_MAX_R 1015
#define MAXLINE  50000

int					connfd, slistenfd, rlistenfd;
char                buf[100];
char                dataBuf[MAXLINE];
pollfd              client[OPEN_MAX_C], sinkfd[OPEN_MAX_R];
int                 nready=0, maxi=0, maxSinki=0, numCon=0, numCli=0;
long long int       counter=0;
double              resetTime=0, curTime=0;
struct timeval      curTV;

void acceptConnect(int i, int conType);
void disconnect(int disi, int conType);
void recCommand(int i);
void addListenPort(int* fd, int port);
void* accept(void* arg);
void* recCommand(void* arg);

void acceptConnect(int connfd, int conType){
    int i;
    if(conType==0){ //command channel 
        for(i=1; i<OPEN_MAX_C-1; i++){
            if(client[i].fd<0){
                client[i].fd = connfd;
                client[i].events = POLLRDNORM | POLLERR;
                if(maxi<i) maxi = i;
                numCli++;
                break;
            }
        }           
    } else if(conType==1){ //datasink channel
        for(i=1; i<OPEN_MAX_R-1; i++){
            if(sinkfd[i].fd<0){
                sinkfd[i].fd = connfd;
                sinkfd[i].events = POLLRDNORM | POLLERR;
                if(maxSinki<i) maxSinki=i;
                numCon++;
                break;
            }
        }  
    } 
    if (conType==0 && i >= OPEN_MAX_C-1){
        // printf("fd: %d\n", connfd);
        printf("too many client!\n");
        close(connfd);
    } else if(conType==1 && i >= OPEN_MAX_R-1){
        printf("too many connection!\n");
        close(connfd);
    }
}

void disconnect(int disi, int conType){
    if(conType==0){
        close(client[disi].fd);
        client[disi].fd = -1;
        numCli--;
    } else{
        close(sinkfd[disi].fd);
        sinkfd[disi].fd = -1;
        numCon--;   
    }
}

void recCommand(int i){
    gettimeofday(&curTV, NULL);
    curTime = curTV.tv_sec*1.0 + curTV.tv_usec*1e-6;

    if(strcmp(buf, "/reset\n")==0){
        resetTime = curTime;
        sprintf(buf, "%f RESET %lld\n", curTime, counter);
        if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
            if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        }
        counter=0;
    } else if(strcmp(buf, "/report\n")==0){
        double etime = curTime-resetTime;
        double rate = (double)counter*8/1000000/etime;
        sprintf(buf, "%f REPORT %fs %fMbps\n", curTime, etime, rate);
        if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
            if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        }
    } else if(strcmp(buf, "/ping\n")==0){
        sprintf(buf, "%f PONG\n", curTime);
        if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
            if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        }
    } else if(strcmp(buf, "/clients\n")==0){
        sprintf(buf, "%f CONNECT %d\n", curTime, numCon);
        if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
            if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        }
        // sprintf(buf, "%f CLIENTS %d\n", curTime, numCli);
        // if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
        //     if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        // }
    } else{
        sprintf(buf, "Unkown Command\n");
        if(send(client[i].fd, buf, strlen(buf), MSG_NOSIGNAL)<0){
            if (errno == ECONNRESET || errno == EPIPE) disconnect(i, 0);
        }
    }
    
}

void addListenPort(int* fd, uint16_t port){
    int reuse = 1;
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse));
    
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(0);
	servaddr.sin_port        = htons(port);
	if(bind(*fd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0)
        perror("bind error");
    if(listen(*fd, OPEN_MAX_R)<0)
        perror("listen error");
}

void* revData(void* arg){
    struct sockaddr_in 	cliaddr;
    socklen_t			clilen= sizeof(cliaddr);
    int i, n, sockfd;
    for(;;){
        poll(sinkfd, maxSinki+1, -1);
        if (sinkfd[0].revents & POLLRDNORM) {	/* new sinkfd connection */
			connfd = accept(rlistenfd, (struct sockaddr *) &cliaddr, &clilen);
            acceptConnect(connfd, 1);
        }
     
        for(i=1; i<=maxSinki; i++){
            if((sockfd = sinkfd[i].fd)<0) continue;
            if(sinkfd[i].revents & (POLLRDNORM | POLLERR)){
                memset(dataBuf, '\0', sizeof(dataBuf));
                if ( (n = read(sockfd, &dataBuf, sizeof(dataBuf))) < 0) {
					if (errno == ECONNRESET) disconnect(i, 1);
					else perror("read error");
				} else if (n == 0) disconnect(i, 1); 
                else{
                    // printf("%s", dataBuf);
                    counter+=n;
                }
            }
        }   
    }
}

void* accept(void* arg){
    
    struct sockaddr_in 	cliaddr;
    socklen_t			clilen= sizeof(cliaddr);
    int i, n, sockfd;
    for(;;){
        poll(client, maxi+1, -1);
        if (client[0].revents & POLLRDNORM) {	/* new client connection */
			connfd = accept(slistenfd, (struct sockaddr *) &cliaddr, &clilen);
            acceptConnect(connfd, 0);
        }
     
        for(i=1; i<=maxi; i++){
            if((sockfd = client[i].fd)<0) continue;
            if(client[i].revents & (POLLRDNORM | POLLERR)){
                memset(buf, '\0', sizeof(buf));
                if ( (n = read(sockfd, &buf, sizeof(buf))) < 0) {
					if (errno == ECONNRESET) disconnect(i, 0);
					else perror("read error");
				} else if (n == 0) disconnect(i, 0);
                else recCommand(i);
            }
        }   
    }
}

int main(int argc, char** argv){
	
    uint16_t sPort = atoi(argv[1]);
    uint16_t rPort = sPort+1;

    addListenPort(&slistenfd, sPort);
    addListenPort(&rlistenfd, rPort);

    for (int i = 0; i < OPEN_MAX_C; i++){
        client[i].fd = -1;
        
    }
    for(int i=0; i< OPEN_MAX_R; i++){
        sinkfd[i].fd = -1;
    }
    

    client[0].fd = slistenfd;
    client[0].events = POLLRDNORM;
    sinkfd[0].fd = rlistenfd;
    sinkfd[0].events = POLLRDNORM;			

    maxi=0;
    maxSinki=0;

    pthread_t t;
    pthread_create(&t,NULL, revData, (void*)"sucess");   // create a child thread

    
    struct sockaddr_in 	cliaddr;
    socklen_t			clilen= sizeof(cliaddr);

    int i, n, sockfd;
    for(;;){
        poll(client, maxi+1, -1);
        if (client[0].revents & POLLRDNORM) {	/* new client connection */
			connfd = accept(slistenfd, (struct sockaddr *) &cliaddr, &clilen);
            acceptConnect(connfd, 0);
        }
     
        for(i=1; i<=maxi; i++){
            if((sockfd = client[i].fd)<0) continue;
            if(client[i].revents & (POLLRDNORM | POLLERR)){
                memset(buf, '\0', sizeof(buf));
                if ( (n = read(sockfd, &buf, sizeof(buf))) < 0) {
					if (errno == ECONNRESET) disconnect(i, 0);
					else perror("read error");
				} else if (n == 0) disconnect(i, 0);
                else recCommand(i);
            }
        }   
    }

    return 0;
}