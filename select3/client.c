#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>

#include "wrap.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666
#define MAXLINE 1024

void str_cli(FILE * fp, int sockfd){
	int maxfdp1, stdineof;
	fd_set rset;
	int fd = fileno(fp);
	char buf[MAXLINE];
	int n;

	stdineof = 0;
	FD_ZERO(&rset);

	while(1)
	{
		if(stdineof == 0)
			FD_SET(fd, &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fd, sockfd) + 1;

		Select(maxfdp1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(sockfd, &rset))
		{
			if( (n = Read(sockfd, buf, MAXLINE)) == 0){
				if(stdineof == 1)
					return;
				else
					perr_exit("str_cli:server terminated prematurely");
			}
			
			Write(STDOUT_FILENO, buf, n);

		}
		if(FD_ISSET(fd, &rset))
		{
			if( (n = Read(fd, buf, MAXLINE)) == 0){
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				FD_CLR(fd, &rset);
				continue;
			}

			Writen(sockfd, buf, n);
		}

		
	}

}

int main(){
	int lfd;
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0 ,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
	Connect(lfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));

	str_cli(stdin, lfd);
	close(lfd);

	return 0;
}
