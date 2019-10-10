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

void eventset(struct myevent_s *ev, int fd, void(*call_back)(int, int, void *), void * arg)
{
	ev->fd = fd;
	ev->cal_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;
	memset(ev->buf, 0, sizeof(ev->buf));
	ev->len = 0;
	ev->last_active = time(NULL);

	return;
}

void eventadd(int efd, int events, struct myevents_s *ev)
{
	struct epoll_event epv = {0, {0}};
	int op;

	epv.data.ptr = ev;
	epv.events = ev->events = events;

	if(ev->status == 1)
		op = EPOLL_CTL_MOD;
	else{
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	Epoll_ctl(efd, op, ev->fd, &epv);

	return;
}
void eventdel(int efd, struct myevent_s *ev)
{
	struct epoll_event epv = {0, {0}};
	if(ev->status != 1)
		return;
	epv.data.ptr = ev;
	ev->status= 0;
	epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

	return;
}

void initListenSocket(int efd, short port)
{
	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	int flag = fcntl(lfd, F_GETFL, 0);
	fcntl(lfd, F_SETFL, flag | O_NONBLOCK);

	/* void eventset(struct myevent_s *ev, int fd, 
	   	void (*call_back)(int, int, void *), void *arg);
	 */
	eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
	/* void eventadd(int efd, int events, struct myevent_s *ev);	 
	 */
	eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);
	
	struct sockaddr_in sin;
	bzero(&sin, sizeof(servaddr));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(SERV_PORT);
	
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(lfd, (struct sockaddr *)& servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENEQ);
	return;

}

void recvdata(int fd, int events, void *arg)
{
	struct myevent_s *ev = (struct myevent_s *)arg;
	int len;

	len  = recv(fd, ev->buf, sizeof(ev->buf), 0);
	eventdel(g_efd, ev);

	if(len > 0)
	{
		ev->len = len;
		ev->buf[len] = '\0';
		printf("c[%d]:%s\n", fd, ev->buf);

		eventset(ev, fd, senddata, ev);
		eventadd(g_efd, EPOLLOUT, ev);
	}else if(len == 0){
		close(ev->fd);
		printf("[fd = %d] pos[%ld], closed\n", fd, ev->g_events);
	}else{
		close(ev->fd);
		printf("recv[fd = %d]error[%d]:%s", fd, errno, strerror(errno));
	}

}
void senddata(int fd, int events; void *arg)
{
	struct myevent_s *ev = (struct myevent_s *)arg;
	lnt len;
	
	len = send(fd, ev->buf, ev->len, 0);

	if(len > 0)
	{
		printf("send[fd = %d], [%d]%s\n", fd, len, ev->buf);
		eventdel(g_efd,ev);
		eventset(ev, fd, recvdata, ev);
		eventadd(g_efd, EPOLLIN, ev);
	}else {
		close(ev->fd);
		eventdel(g_efd, ev);
		printf("send[fd = %d] error %s\n", fd, strerror(errno));
	}

}

void acceptconn(int lfd, int events, void *arg)
{
	struct sockaddr_in cin;
	socklen_t len = sizeof(cin);
	int cfd, i;

	cfd = Accept(lfd, (struct sockaddr*)&cin, &len);
	do
	{
		for(i = 0; i < MAX_EVENTS; i++)
			if(g_events[i].status == 0)
				break;
		if(i == MAX_EVENTS){
			printf("%s: max connect limit[%d]\n", _func_, MAX_EVENTS);
			break;
		}

		int flag = fcntl(cfd, F_GETFL, 0);
		fcntl(cfd, F_GETFL, flag | O_NONBLOCK);

		eventset(&g_events[i], cfd, recvdata, &g_event[i]);
		eventadd(g_efd, EPOLLIN, &g_events[i]);

	}while(0);

	char buf[INET_ADDRSTRLEN];
	printf("new connect [%s:%d][time:%ld], pos[%d]",
			inet_ntop(AF_INET, cfd.sin_addr.s_addr, buf, INET_ADDRSTRLEN),
			ntohs(cin.sin_port),
			g_events.last_active, 
			i);
}

int main()
{
	unsigned short port = SERV_PORT;
	if(argc == 2)
		port = atoi(argv[i]);

	g_efd = Epoll_create(MAX_EVENT + 1);

	initListenSocket(g_efd, port);	

	struct epoll_event events[MAX_EVENTS + 1];

	int checkpos = 0, i;
	while(1)
	{
		long now = time(NULL);
		for(i = 0; i < 100; i++, checkpos++)
		{
			if(checkops == MAX_EVENTS)
				checkpos = 0;
			if(g_events[checkpos].status != 1)
				continue;
			long duration = now - g_events[checkpos].last_active;

			if(duration >= 60)
			{
				close(g_events[checkpos].fd);
				printf("[fd=%d] timeoit\n", g_event[checkpos].fd);
				eventdel(g_efd, &g_events[checkpos]);
			}
		}

		int  nfd = Epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);

		for(i = 0; i < nfds; i++)
		{
			struct myevent_s *ev = (struct myevent_s)events[i].data.ptr;
			if( (events[i].events & EPOLLIN) && (ev->events & EPOLLIN) )
				ev->call_back(ev->fd, events[i].events, ev->arg);
			if( (events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT) )
				ev->call_back(ev->fd, events[i].events, ev->arg);
		}
	}


	return 0;
}
