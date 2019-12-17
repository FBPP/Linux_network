#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <wait.h>
#include "wrap.h"
#include <pthread.h>

#define SERV_IP "127.0.0.1"

#define MAXLINE 8192
#define SERV_PORT 8000

struct s_info{
	struct sockaddr_in clie_addr;
	int connfd;
};
	
void * do_work(void *arg){
	int n,i;
	struct s_info * ts = (struct s_info*)arg;
	int connfd = ts->connfd;
	struct sockaddr_in clie_addr = ts->clie_addr;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];

	while(1)
	{

		n = Read(connfd,buf, MAXLINE);
		if(n == 0){
			printf("the client %d closed...\n",connfd);
			break;
		}
		printf("received from %s at POTR %d\n",
				inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, str, sizeof(str)),
					ntohs(clie_addr.sin_port));
		for(i = 0; i < n; i++)
			buf[i]  = toupper(buf[i]);

		Write(STDOUT_FILENO, buf, n);
		Write(connfd, buf, n);
	}
	Close(connfd);
	return NULL;
}

int main(){

	int lfd, cfd;
	struct sockaddr_in serv_addr, clie_addr;
	
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
	Bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	Listen(lfd, 128);

	socklen_t clie_addr_len;
	clie_addr_len = sizeof(clie_addr);
	pthread_t tid;
	struct s_info ts[256];

	int i = 0;
	while(1)
	{
		cfd = Accept(lfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
		ts[i].clie_addr = clie_addr;
		ts[i].connfd = cfd;


		pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
		pthread_detach(tid);
		i++;
	}
	return 0;
}
