/*
 * Lab problem set for INP course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define err_quit(m) { perror(m); exit(-1); }
#define max_file_size 34000 
#define max_num_file 1000
#define fragment_size 1000
#define header_size 13
#define data_size (fragment_size-header_size)
#define ack_size 100


struct header{
	char filename[6];  //int: 000000-000999
	char fragment[2];  //int: no more than 33
	char file_size[5]; //int: no more than 33KB
};

void readFile();
void fragmentation();
void sendFrag(int file, int frag);
void resent(char *ack_buf);
void clearResentFile();
int receive_ack();

int s;
static struct sockaddr_in sin;
socklen_t sinlen;
int num_file;
char path_file[100];
char file_buf[max_num_file][max_file_size];
int file_size[max_num_file];
int file_frag[max_num_file];
char buf[fragment_size];

char ack_buf[1000];
int ack_num, ack_file_num, ack_file, ack_frag_num, ack_file_frag;
int finish = 0;
int num;
int main(int argc, char *argv[]) {
	if(argc < 3) {
		return -fprintf(stderr, "usage: %s ... <port> <ip>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-2], NULL, 0));
	if(inet_pton(AF_INET, argv[argc-1], &sin.sin_addr) != 1) {
		return -fprintf(stderr, "** cannot convert IPv4 address for %s\n", argv[1]);
	}
	
	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	connect(s, (struct sockaddr*) &sin, sizeof(sin));
	sinlen = sizeof(sin);

	struct timeval timeout={1,0};
    int ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	num_file = atoi(argv[2]);
	sprintf(path_file, "%s", argv[1]);

	readFile();
	fragmentation();
	printf("enter\n");
	//for(int k=0; k<3; k++){ //500
		for(int i=0; i<num_file; i++){
			for(int j=0; j<file_frag[i]; j++){
				//for(int k=0; k<3; k++){
					sendFrag(i, j);
					usleep(100);
				//}
			}
		}
	//}
	printf("leave\n");
	
	for(;;){
		if(recvfrom(s, ack_buf, fragment_size, 0, (struct sockaddr*) &sin, &sinlen) < 0){
				if(errno == EAGAIN){
					break;
				}//printf("timeout\n");
				else
					perror("receve: ");
		}
		else{
			if(strncmp(ack_buf, "finish", 6) == 0){
				printf("receive: finish\n");
				break;
	        }
			//for(int i=0; i<1; i++){
				resent(ack_buf);
			//}
		}
	}
	close(s);
}

void sendFrag(int file, int frag){
	//memset(buf, '\0', fragment_size);
	sprintf(buf, "%06d%02d%05d", file, frag, file_size[file]); //header
    memcpy(buf+header_size, file_buf[file]+frag*data_size, data_size); //data

	//printf("strlen(buf) = %ld\n", strlen(buf));
	if(frag == file_frag[file]-1)
	    send(s, buf, strlen(buf), MSG_NOSIGNAL);
    else 
        send(s, buf, fragment_size, MSG_NOSIGNAL);
	//printf("sendSize = %d\n", sendSize);
}

void readFile(){
	char path[120];
	int file;
	int size;

	for(int i=0; i<num_file; i++){
		sprintf(path, "%s/%06d", path_file, i);

		if((file = open(path, O_RDONLY))<0){
            perror("open error");
        }
		if((size = read(file, &file_buf[i], max_file_size))<0){
            perror("read error\n");
        }
		// printf("%s", file_buf[i]);
		file_size[i] = size;
	}
}

void fragmentation(){
	for(int i=0; i<num_file; i++){
		file_frag[i] = file_size[i]/data_size;
		if(file_size[i]%data_size>0) file_frag[i]++;
		file_size[i] += file_frag[i]*header_size;
	}
}

void resent(char *ack_buf){
    sscanf(ack_buf, "%03d%02d", &ack_file, &ack_frag_num);
    if(ack_frag_num == 0){
        for(int j=0; j<file_frag[ack_file]; j++){
            sendFrag(ack_file, j);
        }
    } 
    for(int j=0; j<ack_frag_num; j++){
        sscanf(ack_buf+5 +j*2, "%02d", &ack_file_frag);
        sendFrag(ack_file, ack_file_frag);
    }
}
