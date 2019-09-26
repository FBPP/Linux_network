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

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666
#define MAXLINE 1024
#define LISTENEQ 128
#define OPEN_MAX 1024

int main()
{
	int  i, maxi, listenfd, connfd, sockfd;	
	int nready;
	ssize_t n;
	char buf[MAXLINE];
	struct pollfd  client[OPEN_MAX];
	struct sockaddr_in  cliaddr, servaddr;
	socklen_t clilen;
	
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(listenfd, (struct sockaddr *)& servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENEQ);

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for(i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1;

	maxi = 0;
	while(1)
	{
		nready = Poll(client, maxi + 1, -1);
		
		if(client[0].revents & POLLRDNORM)
		{
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
			for(i = 1; i < OPEN_MAX; i++)
				if(client[i].fd < 0) {
					client[i].fd = connfd;
					break;
				}

			if(i == OPEN_MAX)
				perr_exit("too many clients");
			client[i].events = POLLRDNORM;
			if(i > maxi)
				maxi = i;
			if(--nready <= 0)
				continue;
		}

		for(i = 1; i <= maxi; i++)
		{
			if( (sockfd = client[i].fd) < 0)
				continue;
			if(client[i].revents & (POLLRDNORM | POLLERR)) 
			{
				if( (n = Read(sockfd, buf, MAXLINE)) < 0) {
					if(errno == ECONNRESET){
						Close(sockfd);
						client[i].fd = -1;
					} else
						perr_exit("read error");
				}
				else if(n == 0) {
					Close(sockfd);
					client[i].fd = -1;
				} else {
					Writen(STDOUT_FILENO, buf, n);
					for(int j = 0; j < n; j++)
						buf[j] = toupper(buf[j]);

					Writen(sockfd, buf, n);
				}
				if(--nready <= 0)
					break;
			}
		}
	}

	return 0;
}
