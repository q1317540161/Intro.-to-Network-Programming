#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>

using namespace std;

#define buf_size 100
#define board_size 9
#define num_hole 30

void readBoard();
void dealBoard();
void sendBoard(int sockfd);
bool recDealBoard(int order);
void setValue(int inI, int inJ, int inB, int value, bool set);
bool checkValid(int inI, int inJ, int inB, int value);
void printBoard();

char sendBuf[buf_size];
char recvBuf[buf_size];
int  board[board_size][board_size];
vector<pair<int, int>> holeLoc;

bool rowValid[board_size][board_size+1]; 
bool colValid[board_size][board_size+1];
bool boxValid[board_size][board_size+1];

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_un	servaddr;
	char path[] = "/sudoku.sock";

	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, path);

	memset(rowValid, 1, sizeof(rowValid));
	memset(colValid, 1, sizeof(colValid));
	memset(boxValid, 1, sizeof(boxValid));

	if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0)
		perror("connect error");
	if(write(sockfd, "S\n", strlen("S\n"))<0)
		perror("write error");
	if(read(sockfd, recvBuf, buf_size)<0)
		perror("read error");
	else{
		// printf("recv:%s\n", recvBuf);
		readBoard();
		dealBoard();
		sendBoard(sockfd);

		if(write(sockfd, "C\n", strlen("C\n"))<0)
			perror("write error");
	}

	exit(0);
}

void sendBoard(int sockfd){
	int inI, inJ;
	for(int i=0; i<num_hole; i++){
		memset(sendBuf, '\0', sizeof(sendBuf));
		inI = holeLoc[i].first;
		inJ = holeLoc[i].second;
		sprintf(sendBuf, "V %d %d %d\n", inI, inJ, board[inI][inJ]);
		
		if(write(sockfd, sendBuf, 8)<0)
			perror("write error");
		if(read(sockfd, recvBuf, buf_size)<0)
			perror("read error");
	}
}


void readBoard(){
	int b;
	for(int i=0; i<board_size; i++){
		for(int j=0; j<board_size; j++){
			if(recvBuf[4+j+board_size*i]=='.')
				holeLoc.push_back(make_pair(i, j));
			else{
                board[i][j] = (int)(recvBuf[4+j+board_size*i]-'0');
				b = (j/3) + 3*(i/3);
				setValue(i, j, b, board[i][j], true);
			}
		}
	}
}

void dealBoard(){
	recDealBoard(0);
}

bool recDealBoard(int order){
	int inI = holeLoc[order].first;
	int inJ = holeLoc[order].second;
	int inB = (inJ/3)+3*(inI/3);
    
	int chosen;
	bool valid=false;	

	if(order==(num_hole-1)){
		for(chosen=1; chosen<=board_size; chosen++){
			valid = checkValid(inI, inJ, inB, chosen);
			if(valid){
                setValue(inI, inJ, inB, chosen, true);
                break;
            }
		}
		return valid;
	}
	
	for(chosen=1; chosen<=board_size; chosen++){
		if(checkValid(inI, inJ, inB, chosen)){
			setValue(inI, inJ, inB, chosen, true);

			if(valid = recDealBoard(order+1)) break;
			else setValue(inI, inJ, inB, chosen, false);
		}
	}
	return valid;
}

void setValue(int inI, int inJ, int inB, int value, bool set){
	if(set){
        board[inI][inJ] = value;
		rowValid[inI][value]=false;
		colValid[inJ][value]=false;
		boxValid[inB][value]=false;
	} else{
        board[inI][inJ] = 0;
		rowValid[inI][value]=true;
		colValid[inJ][value]=true;
		boxValid[inB][value]=true;
	}
}

bool checkValid(int inI, int inJ, int inB, int value){

	return ((rowValid[inI][value])&
			(colValid[inJ][value])&
			(boxValid[inB][value]));
}