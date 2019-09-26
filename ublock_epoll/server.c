#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include "wrap.h"
#include <limits.h>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666
#define MAXLINE 10
#define LISTENEQ 128
#define OPEN_MAX 10

int main()
{
	int  listenfd, connfd;
	ssize_t n, epfd;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	struct pollfd;  
	struct sockaddr_in  cliaddr, servaddr;
	socklen_t clilen;

	struct epoll_event event, resevents[OPEN_MAX];
	
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(listenfd, (struct sockaddr *)& servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENEQ);
	
	epfd = Epoll_create(OPEN_MAX);
	clilen = sizeof(cliaddr);

	connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
	printf("received from %s at port %d\n",
			inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, str, sizeof(str)),
			ntohs(cliaddr.sin_port)
			);

 	int flag = fcntl(connfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(connfd, F_SETFL, flag);

	event.data.fd = connfd;
	event.events = EPOLLIN | EPOLLET; //ET
	Epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);

	while(1)
	{
		printf("epoll_wait begin\n");
		Epoll_wait(epfd, resevents, OPEN_MAX, -1);
		printf("epoll_wait end\n");

		if(resevents[0].data.fd == connfd)
			while( (n = Read(connfd, buf, MAXLINE/2)) > 0)
				write(STDOUT_FILENO, buf, n);
	}
	

	Close(listenfd);
	Close(epfd);
	return 0;
}
