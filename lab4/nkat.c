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
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>

void sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return;
}

int main(int argc, char **argv)
{
    int					sockfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int scale=1, port=0;
    for(int i=strlen(argv[1])-1; i>=0; i--){
        int tmp = argv[1][i] - '0';
        port+=tmp*scale;
        scale*=10;
    }
    printf("port:%d\n", port);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(0);
	servaddr.sin_port        = htons(port);

	if(bind(sockfd, (struct sockadd_in *) &servaddr, sizeof(servaddr))<0)
        perror("bind error");

	listen(sockfd, 20);

	signal(SIGCHLD, sig_chld);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(sockfd, (struct sockaddr_in *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				perror("accept error");
		}
        else printf("New connection from %s:%d\n",inet_ntoa(cliaddr.sin_addr),  cliaddr.sin_port);

		if ( (childpid = fork()) == 0) {	/* child process */
			close(sockfd);	/* close listening socket */
			//mycode
			int stdi = dup(0);
            int stdo = dup(1);
			int stde = dup(2);

			dup2(connfd, STDIN_FILENO);
            dup2(connfd, STDOUT_FILENO);
			dup2(connfd, STDERR_FILENO);
            
            if(execvp(argv[2], argv+2)<0)
				perror("execute error");

            //
			exit(0);
		}

		close(connfd);			/* parent closes connected socket */
	}


    return 0;
}