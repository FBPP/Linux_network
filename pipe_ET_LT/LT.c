#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#include "wrap.h"

#define MAXLINE 10
#define OPEN_MAX 10

int main()
{
	int efd, i;
	int pfd[2];
	pid_t pid;
	char buf[MAXLINE], ch = 'a';
	int res;

	pipe(pfd);
	pid = fork();

	if(pid == 0)
	{
		Close(pfd[0]);
		while(1)
		{
			for(i = 0; i < MAXLINE/2; i++)
				buf[i] = ch;
			buf[i - 1] = '\n';
			ch++;
			for(; i < MAXLINE; i++)
				buf[i] = ch;
			buf[i - 1] = '\n';
			ch++;
			Write(pfd[1], buf, sizeof(buf));
			sleep(3);
		}
		close(pfd[1]);
	} else if(pid > 0){
		struct epoll_event event, resevents[OPEN_MAX];
		int len;

		Close(pfd[1]);
		efd = Epoll_create(OPEN_MAX);

		event.events = EPOLLIN;          //LT 
		//event.events = EPOLLIN | EPOLLET //ET
		event.data.fd = pfd[0];
		Epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);
		
		while(1)
		{
			res = Epoll_wait(efd, resevents, OPEN_MAX, -1);
			printf("%d\n", res);
			if(resevents[0].data.fd == pfd[0]){
				len = Read(pfd[0], buf, MAXLINE/2);
				Write(STDOUT_FILENO, buf, len);
			}
		}
		close(pfd[0]);
		close(efd);

	} else{
		perror("fork");
		exit(-1);
	}
	return 0;
}
