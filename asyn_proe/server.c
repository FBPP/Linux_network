#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <wait.h>
#include "wrap.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

void  wait_child(int signo){
	
	while(waitpid(0, NULL, WNOHANG) > 0);

	return ;
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
	pid_t pid;

	char buf[BUFSIZ], clie_IP[BUFSIZ];
	int n, i;
	while(1)
	{
		cfd = Accept(lfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
		printf("client IP%s, potr:%d\n", inet_ntop(AF_INET,
				       	&clie_addr.sin_addr.s_addr,
				       clie_IP,sizeof(clie_IP)), 
				ntohs(clie_addr.sin_port));

		pid  = fork();
		if(pid < 0)
		{
			perror("fork error");
			exit(1);
		} else if (pid == 0){
			break;
		} else{
			close(cfd);
			signal(SIGCHLD, wait_child);
		}
	}
	if(pid == 0){
		close(lfd);
		while(1)
		{
			n = Read(cfd, buf, sizeof(buf));
			if(n == 0)
			{
				close(cfd);
				return 0;
			} else if(n == -1){
				perror("read error");
				exit(1);
			} else{
				for(i = 0; i < n; i++)
					buf[i] = toupper(buf[i]);
				write(cfd, buf, n);
			}
		}
	} 

	return 0;
}
