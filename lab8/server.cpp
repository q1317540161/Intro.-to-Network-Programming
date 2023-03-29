#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#define err_quit(m) { perror(m); exit(-1); }
#define max_file_size 34000 
#define max_num_file 1000
#define fragment_size 1000
#define header_size 13
#define data_size (fragment_size-header_size)
#define max_num_ack 72
#define max_ack_file 14
#define ack_size 100

struct recvHeader{
	int  filename;  	//6byte int: 000000-000999
	int  fragment;		//2byte int: no more than 33
	int  file_size;		//5byte int: no more than 33KB
};

struct ack{ 			//at most 72 ack package
	int file_size;		//2byte int: limit 14 file

	// per file
	int filename;  		//3byte int: 000-999
	int fragment_num;  	//2byte int: 00-33
};

void readFile(int rLen);
void writeFile(int file, int size);
void ack(int s, struct sockaddr_in &csin);
//void ack(int sig);
//void call_ack(int sig);

// int s;
// struct sockaddr_in sin;
int num_file;
char path_file[100];
char path[120];
char file_buf[max_num_file][max_file_size];
int file_frag_num[max_num_file]={0};
bool file_recv_frag[max_num_file][34];
int file_recv_byte[max_num_file]={0};
bool file_done[max_num_file]={0};

char recv_buf[fragment_size];
char send_buf[max_num_file*(5+100)]; //to send which fragment haven't been received
int file, frag, tmp_file_size;

char ack_buf[max_num_file][ack_size];
char ack_pkg[fragment_size];
int ack_index=0;
bool flag_have_client = 0;
int recv_file_num = 0;
int finish = 0;
char ack_data[100];

int s;
struct sockaddr_in sin;
struct sockaddr_in csin;
int main(int argc, char *argv[]) {
	// int s;
	// struct sockaddr_in sin;
	if(argc < 2) {
		return -fprintf(stderr, "usage: %s ... <port>\n", argv[0]);
	}

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(strtol(argv[argc-1], NULL, 0));

	if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		err_quit("socket");

	if(bind(s, (struct sockaddr*) &sin, sizeof(sin)) < 0)
		err_quit("bind");

	num_file = atoi(argv[2]);
	sprintf(path_file, "%s", argv[1]);

	struct timeval timeout={0,5};
    int ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	
	//struct sockaddr_in csin;
	socklen_t csinlen = sizeof(csin);
	int rlen;
	printf("path = %s\n", argv[1]);
	for(int i=0; i<num_file; i++){
		memset(file_buf[i], '\0', max_file_size);
	}
	
	while(1) {
		if(recv_file_num == max_num_file) break;
		memset(recv_buf, '\0', fragment_size);

		if((rlen = recvfrom(s, recv_buf, fragment_size, 0, (struct sockaddr*) &csin, &csinlen)) < 0) {
			if(errno = EAGAIN && flag_have_client){
				//printf("ack\n");
				ack(s, csin);
			}
			else if(flag_have_client) perror("recvfrom");
		}
		if(rlen>0){
			flag_have_client = true;
			//printf("recv size= %d\n", rlen);
			readFile(rlen);
			// printf("%s", recv_buf);
			//printf("size: %d\nbuf: %s\n", rlen, recv_buf);
		}
	}
	for(int i=0; i<5; i++)
		if(sendto(s, "finish", strlen("finish"), 0, (struct sockaddr*) &csin, sizeof(csin)) < 0)
			perror("sendto");
	printf("finish\n");
	close(s);
}

void readFile(int rLen){
	sscanf(recv_buf, "%06d%02d%05d", &file, &frag, &tmp_file_size);
	memcpy(file_buf[file]+frag*data_size, recv_buf+header_size, rLen-header_size);

	if(!file_recv_frag[file][frag])
		file_recv_byte[file] += rLen;

	if(!file_frag_num[file]){
		file_frag_num[file] = tmp_file_size / fragment_size;
		if(tmp_file_size % 1000) file_frag_num[file]++;
	}
	//printf("file buffer =\n%s\n", file_buf[file]+frag*data_size);

	file_recv_frag[file][frag]=true;
	
	if(file_recv_byte[file]==tmp_file_size && !file_done[file]){
		writeFile(file, (tmp_file_size - header_size*file_frag_num[file]));
		file_done[file]=true;
		recv_file_num++;
		printf("%d write %d success\n", recv_file_num, file);
	}
		
}

void writeFile(int file, int size){
	sprintf(path, "%s/%06d", path_file, file);
	int sockfd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0777);
	
	if(write(sockfd, file_buf[file], size) < 0)
		perror("write error");
	close(sockfd);
}


void ack(int s, struct sockaddr_in &csin){ //int s, struct sockaddr_in &csin
	int frag_num;
	for(int i=0; i<num_file; i++){
		frag_num = 0;
		if(!file_done[i]){
			for(int j=0; j<file_frag_num[i]; j++){
				if(!file_recv_frag[i][j]){
					sprintf(ack_data + 2*frag_num, "%02d", j);
					frag_num++;
				}
			}
			sprintf(ack_pkg , "%03d%02d%s", i, frag_num, ack_data);
			if(sendto(s, ack_pkg, ack_size, 0, (struct sockaddr*) &csin, sizeof(csin)) < 0)
					perror("sendto");
		}
	}
}