/* include fig01 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<sys/wait.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h> 
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<poll.h>
#include<limits.h>		/* for OPEN_MAX */
#include<time.h>
#include<stdbool.h>
#include"aa.h"

#define MAXLINE 	2000
#define MAXMES  	2500
#define OPEN_MAX 	1024

struct user{
	char name[20];
	char ipAddr[15];
	int  port;
};

void seeOnline(int askUser);
void broadcast(char* message, int sender, bool except);
void disconnect(int i);
void cur_time(char* input);

int					i, maxi, listenfd, connfd, sockfd;
int					nready;
ssize_t				n;
char				buf[MAXLINE];
socklen_t			clilen;
struct pollfd		client[OPEN_MAX];
struct sockaddr_in	cliaddr, servaddr;
struct user			userData[OPEN_MAX];
char				curTime[30];
char				message[MAXMES];
int					numCli = 0;
int					lostUser[OPEN_MAX];
int					numLost = 0;

void seeOnline(int askUser){
	int sockfd = client[askUser].fd;
	if(send(sockfd, "--------------------------------------------------\n", 51, MSG_NOSIGNAL)<0){
		if (errno == ECONNRESET || errno == EPIPE) disconnect(askUser);
		return;
	}
	for(int i=1; i<=maxi; i++){
		if(i==askUser){
			sprintf(message, "* %-20s %s:%d\n", userData[i].name,  userData[i].ipAddr,  userData[i].port);
			if(send(sockfd, message, strlen(message), MSG_NOSIGNAL)<0){
				if (errno == ECONNRESET || errno == EPIPE){
					disconnect(askUser);
					return;
				} 
			}
		}
		else if(client[i].fd>0){
			sprintf(message, "  %-20s %s:%d\n", userData[i].name,  userData[i].ipAddr,  userData[i].port);
			if(send(sockfd, message, strlen(message), MSG_NOSIGNAL)<0){
				if (errno == ECONNRESET || errno == EPIPE) {
					disconnect(askUser);
					return;
				}
			}
		}
	}
	if(send(sockfd, "--------------------------------------------------\n", 51, MSG_NOSIGNAL)<0){
		if (errno == ECONNRESET || errno == EPIPE) {
			disconnect(askUser);
			return;
		}
	}
}
void broadcast(char* message, int sender, bool except){
	for(int i=1; i<=maxi; i++){
		if(except && i==sender) continue;
		if(client[i].fd<0) continue;
		if(send(client[i].fd, message, strlen(message), MSG_NOSIGNAL)<0)
			if (errno == ECONNRESET || errno == EPIPE) disconnect(i);
	}
}
void disconnect(int disi){
	close(client[disi].fd);
	client[disi].fd = -1;
	numCli--;
	lostUser[numLost]=disi;
	numLost++;
	printf("* %d client %s:%d disconnected\n", disi, userData[disi].ipAddr, userData[disi].port);
	sprintf(message, "%s *** User <%s> has left the server\n", curTime, userData[disi].name);
	broadcast(message, disi, 1);
}

void cur_time(char* input){
    struct tm* cur;
    time_t t;
    t=time(NULL);
    cur = localtime(&t);
    
    sprintf(input, "%-4d-%-2d-%-2d %-2d:%-2d:%-2d",cur->tm_year+1900, cur->tm_mon+1, cur->tm_mday, cur->tm_hour, cur->tm_min, cur->tm_sec);    
}

int main(int argc, char **argv)
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	int scale=1, port=0;
    for(int i=strlen(argv[1])-1; i>=0; i--){
        int tmp = argv[1][i] - '0';
        port+=tmp*scale;
        scale*=10;
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if(bind(listenfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr))<0)
		perror("bind error");

    int LISTENQ;
	listen(listenfd, OPEN_MAX);

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array */
    numCli = 0;             /* num of clients */
/* end fig01 */

/* include fig02 */
	for ( ; ; ) {
		cur_time(curTime);
		nready = poll(client, maxi+1, -1);

		if (client[0].revents & POLLRDNORM) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr_in *) &cliaddr, &clilen);

			for (i = 1; i < OPEN_MAX-1; i++)
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
			if (i >= OPEN_MAX-1){
                printf("too many client!\n");
				//write(connfd, "Connection failed. The chatroom is full now.\n",sizeof("Sorry, the connection is full.\n"));
				close(connfd);
            }
            else{
				numCli++;
				client[i].events = POLLRDNORM | POLLERR;

				int r1 = rand() % sizeAnimals;
				int r2 = rand() % sizeAdjs;
				sprintf(userData[i].name, "%s %s", adjs[r1], animals[r2]);
				sprintf(userData[i].ipAddr, "%s", inet_ntoa(cliaddr.sin_addr));
				userData[i].port = cliaddr.sin_port;

				sprintf(message, "%s *** Welcome to the simple CHAT server\n%s *** Total %d users online now. Your name is %s\n", curTime, curTime, numCli, userData[i].name);
				if(send(connfd, message, strlen(message), MSG_NOSIGNAL)<0){
					if (errno == ECONNRESET || errno == EPIPE) disconnect(i);
				}
				else{
					printf("* %d client connected from %s:%d\n", i, userData[i].ipAddr, userData[i].port);
					sprintf(message, "%s *** User <%s> has just landed on the server\n", curTime, userData[i].name);
					broadcast(message, i, 1);
					if (i > maxi)	maxi = i;		/* max index in client[] array */
				}
			}
			--nready;
			// if (--nready <= 0) continue;				/* no more readable descriptors */			
		}

		for (i = 1; i <= maxi; i++) {	/* check all clients for request */
			if ( (sockfd = client[i].fd) < 0) continue;
			if (client[i].revents & (POLLRDNORM | POLLERR)) {
				memset(buf, '\0', sizeof(buf));
				if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
					if (errno == ECONNRESET) disconnect(i);
					else perror("read error");
				} else if (n == 0) {
					disconnect(i);
				} else {
					// printf("buf: %s", buf);
					if(buf[0]=='/'){
						int cut=0;
						char command[10]="";
						char tmpName[20]="";
						bool unknown = true;

						for(cut=0;cut<strlen(buf);cut++){
							if(buf[cut]==' ' || buf[cut]=='\n') break;
							command[cut]=buf[cut];
						}
						if(strcmp(command, "/name")==0 && cut<strlen(buf)-1){
							// strncpy(tmpName, &buf[cut], strlen(buf)-cut-1);
							int lenName=0;
							for(cut=cut+1; cut<strlen(buf); cut++){
								if(buf[cut]=='\n') break;
								tmpName[lenName]=buf[cut];
								lenName++;
							}
							if(strlen(tmpName)!=0){
								unknown = false;
								sprintf(message, "%s *** Nickname changed to <%s>\n", curTime, tmpName);
								if(send(sockfd, message, strlen(message), MSG_NOSIGNAL)<0){
									if (errno == ECONNRESET || errno == EPIPE) disconnect(i);
								}
								else{
									sprintf(message, "%s *** User <%s> renamed to <%s>\n", curTime, userData[i].name, tmpName);
									sprintf(userData[i].name, "%s", tmpName);
									broadcast(message, i, 1);
								} 
								
							}
						} else if(strcmp(command, "/who")==0){
							unknown = false;
							seeOnline(i);
						} 
						if(unknown){
							sprintf(message, "%s *** Unknown or incomplete command <%s>\n", curTime, command);
							if(send(sockfd, message, strlen(message), MSG_NOSIGNAL)<0) {
								if (errno == ECONNRESET || errno == EPIPE) disconnect(i);
							}
						}
					} else {
						sprintf(message, "%s <%s> %s", curTime, userData[i].name, buf);
						broadcast(message, i, 1);
					}
				}
				if (--nready <= 0) break;				/* no more readable descriptors */
			}
		}
	}
}
/* end fig02 */
