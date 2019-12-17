#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include "wrap.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666
#define MAXLINE 1024
#define LISTENEQ 128

int main()
{
	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t clien;
	struct sockaddr_in cliaddr, servaddr;
		
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENEQ);

	maxfd = listenfd; 
	maxi = -1;
	for(i = 0; i < MAXLINE; i++)
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	while(1)
	{
		rset = allset;
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(listenfd, &rset))
		{
			clien = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clien);

			for(i = 0; i < FD_SETSIZE; i++)
				if(client[i] < 0) {
					client[i] = connfd;
					break;
				}

			if(i == FD_SETSIZE)
				perr_exit("too many clients");

			FD_SET(connfd, &allset);
			if(connfd > maxfd)
				maxfd = connfd;
			if(i > maxi)
				maxi = i;
			if(--nready <= 0)
				continue;
		}

		for(i = 0; i <= maxi; i++)
		{
			if( (sockfd = client[i]) < 0)
				continue;
			if(FD_ISSET(sockfd, &rset))
			{
				if( (n = Read(sockfd, buf, MAXLINE)) == 0) {
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
				} else{
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
