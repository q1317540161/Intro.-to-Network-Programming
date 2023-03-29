#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>
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

static struct timeval _t0;
static unsigned long long bytesent = 0;

double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

//to calculate the sending speed
void handler(int s) {
	struct timeval _t1;
	double t0, t1;
	gettimeofday(&_t1, NULL);
	t0 = tv2s(&_t0);
	t1 = tv2s(&_t1);
	fprintf(stderr, "\n%lu.%06lu %llu bytes sent in %.6fs (%.6f Mbps; %.6f MBps)\n",
		_t1.tv_sec, _t1.tv_usec, bytesent, t1-t0, 8.0*(bytesent/1000000.0)/(t1-t0), (bytesent/1000000.0)/(t1-t0));
	exit(0);
}

#define HEADER_SIZE 66
#define PACKETSIZE 500-HEADER_SIZE

int main(int argc, char *argv[]) {

	signal(SIGINT,  handler);
	signal(SIGTERM, handler);

	//calculate the rate
	unsigned long long int datarate = 0;

	for(int i=0;i<strlen(argv[1]);i++){
		int tmp=0;
		if(i==0){
			tmp = argv[1][i] -'0';
			datarate+=tmp*1000000;
		}
		else if(i==1) continue;
		else if(i==2){
			tmp += argv[1][i] -'0';
			datarate+=tmp*100000;
		}
		else if(i==3){
			tmp += argv[1][i] -'0';
			datarate+=tmp*10000;
		}
	}
	printf("datarate: %lld\n", datarate);
	//connection
	pid_t sockfd;
	struct sockaddr_in	servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(10003);

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("socket error");

	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0)
		perror("inet_pton error");

	if ( (connect(sockfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr))) < 0)
		perror("connect error");
	

	struct timeval _tstart, _tcur;
	double tstart, tcur, t0;

	char* s[PACKETSIZE];
	memset(s, 'a', sizeof(s));
	
	gettimeofday(&_t0, NULL);
	t0 = tv2s(&_t0);

	struct timespec st1 = { 0, 2000};
	struct timespec st2 = { 0, 5000};
	while(1) {

		// gettimeofday(&_tstart, NULL);
		// tstart = tv2s(&_tstart);
		// unsigned long long int bytein1 = 0;

		// while(bytein1<datarate){
		// 	write(sockfd, &s, PACKETSIZE);
		// 	bytein1+=PACKETSIZE+HEADER_SIZE;
		// 	nanosleep(&st2, NULL);
		// }

		// gettimeofday(&_tcur, NULL);
		// tcur = tv2s(&_tcur);

		// usleep((tcur-tstart)*1000000);

		gettimeofday(&_tcur, NULL);
		tcur = tv2s(&_tcur);
		
		if(bytesent/(tcur-t0)<datarate){
			if(write(sockfd, &s, PACKETSIZE)<0)
				printf("Write Error\n");
			else bytesent+=PACKETSIZE+HEADER_SIZE;
			
			// nanosleep(&st1, NULL);
		}
		else{
			nanosleep(&st2, NULL);
		}
		
	}

	return 0;
}
