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

int main(int argc, char **argv)
{   
  pid_t sockfd;
  struct sockaddr_in	servaddr;

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port   = htons(10002);

  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    perror("socket error");
  
  if (inet_pton(AF_INET, "140.113.213.213", &servaddr.sin_addr) <= 0)
    perror("inet_pton error");

  if ( (connect(sockfd, (struct sockaddr_in *) &servaddr, sizeof(servaddr))) < 0)
    perror("connect error");

  char message[100];
  bzero(message, sizeof(message));
  if(read(sockfd, &message, 100)==-1)
    printf("Read Error\n");
  else printf("%s", message);
  
  //tell the server to start sending
  if(write(sockfd, "GO\n", 3)<0)
        printf("Write Error\n");

  //read data
  int count=0;
  char c;
  while(1){
    if(read(sockfd, &c, sizeof(char))==-1)
      printf("Read Error\n");
    if(c=='\n') break;
  }
  while(1){
    if(read(sockfd, &c, sizeof(char))==-1)
      printf("Read Error\n");
    if(c=='\n') break;
    count++;
  }
  while(1){
    if(read(sockfd, &c, sizeof(char))==-1)
      printf("Read Error\n");
    if(c=='\n') break;
  }

  //how many data received?
  bzero(message, sizeof(message));
  if(read(sockfd, &message, 100)==-1)
    printf("Read Error\n");
  else printf("%s", message);

  int tmp;
  char s[15]="";
  char s1[15]="";
  for(int i=0;count!=0;i++){
    tmp=count%10;
    s[i]=tmp+'0';
    count/=10;
  }
  for(int i=0; i<strlen(s);i++){
    s1[i] = s[strlen(s)-i-1];
  }
  s1[strlen(s1)] ='\n';
  printf("count:%s\n", s1);

  if(write(sockfd, &s1, strlen(s1))<0)
    printf("Write Error");

  bzero(message, sizeof(message));
  if(read(sockfd, &message, 100)==-1)
    printf("Read Error\n");
  else printf("%s", message);

  close(sockfd);

  return 0;
}