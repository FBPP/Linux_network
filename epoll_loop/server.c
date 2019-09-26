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

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

#define MAX_EVENTS 1024
#define BUFLEN 4096

struct myevent_s{
	int fd;
	int events;
	void* arg;
	void(*call_back)(int fd, int events, void* arg);
	int status;
	char buf[BUFLEN];
	int len;
	long last_active;
};
int g_efd; //root Node in Red Black Tree
struct myevent_s g_events[MAX_EVENTS + 1];
int main()
{
	int  i, listenfd, connfd, sockfd;	
	int nready;
	int num = 0;
	ssize_t n, epfd;
	char buf[MAXLINE], str[INET_ADDRSTRLEN];
	struct pollfd;  
	struct sockaddr_in  cliaddr, servaddr;
	socklen_t clilen;

	struct epoll_event event, events[OPEN_MAX];
	
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
	event.events = EPOLLIN;
	event.data.fd = listenfd;
	Epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

	while(1)
	{
		nready = Epoll_wait(epfd, events, OPEN_MAX, -1);
		for(i = 0; i < nready; i++)
		{
			if(!events[i].events & EPOLLIN)
				continue;
			if(events[i].data.fd == listenfd)
			{
				clilen = sizeof(cliaddr);
				connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

				printf("received from  %s at port %d\n",
						inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr,str,sizeof(str)), 
						ntohs(cliaddr.sin_port));
				printf("cfd %d --- client %d\n", connfd, ++num);

				event.events = EPOLLIN;
				event.data.fd = connfd;
				Epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event); 
			} else{
				sockfd = events[i].data.fd;
				n = Read(sockfd, buf, MAXLINE);
				if(n == 0)
				{
					Epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
					Close(sockfd);
					printf("client[%d] closed connection\n", sockfd);
				} else if(n < 0){
					perror("read < 0 error:");
					Epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
					Close(sockfd);
				} else{
					for(i = 0; i < n; i++)
						buf[i] = toupper(buf[i]);
					Write(STDOUT_FILENO, buf, n);
					Write(sockfd, buf, n);
				}
			}
		}
	}
	Close(listenfd);
	Close(epfd);
	return 0;
}
