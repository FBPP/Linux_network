#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/epoll.h>
#include "wrap.h"

void perr_exit(const char * s){
	perror(s);
	exit(-1);
}

int Accept(int fd, struct sockaddr * sa, socklen_t * addrlen){
	int n;
again:
	if((n = accept(fd, sa, addrlen)) < 0)
	{
		if((errno == ECONNABORTED) || (errno == EINTR))
			goto again;
		else
			perr_exit("accept error");
	}
	return n;
}

int Bind(int fd, const struct sockaddr *sa, socklen_t addrlen){
	int n;
	if((n = bind(fd, sa, addrlen)) < 0)
		perr_exit("bind error");
	return n;
}

int Connect(int fd, const struct sockaddr * sa, socklen_t salen){
	int n;
	printf("-----------------------in wrap before connect()\n");
	n = connect(fd, sa, salen);
	printf("-----------------------in wrap connect() return %d\n", n);
	if(n < 0)
		perr_exit("connect error");
	return n;
}

int Listen(int fd, int backlog){
	int n;
	if((n = listen(fd, backlog)) < 0)
		perr_exit("listen error");
	return n;
}

int Socket(int family, int  type, int protocol){
	int n;
	if((n = socket(family, type, protocol)) < 0)
		perr_exit("socket error");
	return n;
}
ssize_t Read(int fd, void * ptr, size_t nbytes){
	ssize_t n;
again:
	if( (n = read(fd, ptr, nbytes)) == -1 )
	{
		if(errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}
ssize_t Write(int fd, const void *ptr, size_t nbytes){
	ssize_t n;
again:
	if( (n = write(fd, ptr, nbytes)) == -1)
	{
		if(errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}
int Close(int fd){
	int n;
	if( (n = close(fd)) == -1)
		perr_exit("close error");
	return n;
}
ssize_t Readn(int fd, void * vptr, size_t n){
	size_t nleft;    //remaining unread bytes number
	ssize_t nread;   //actually read bytes number
	char * ptr;

	ptr = (char *)vptr;
	nleft = n;
	while(nleft > 0)
	{
		if( (nread = read(fd, ptr, nleft)) < 0)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;
		}else if(nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;

}
ssize_t Writen(int fd, const void * vptr, size_t n){
	size_t nleft;
	ssize_t nwritten;
	const char * ptr;

	ptr = (char *)vptr;
	nleft = n;
	while(nleft > 0)
	{
		if( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}
static ssize_t my_read(int fd, char * ptr){
	static int read_cnt;
	static char * read_ptr;
	static char read_buf[100];

	if(read_cnt <= 0)
	{
again:
		if( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
		{
			if(errno == EINTR)
				goto again;
			return -1;
		}else if(read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}
ssize_t Readline(int fd, void *vptr, size_t maxlen){
	ssize_t n, rc;
	char c, * ptr;
	ptr = (char *)vptr;

	for(n = 1; n < maxlen; n++)
	{
		if( (rc = my_read(fd, &c)) == 1)
		{
			*ptr++ = c;
			if(c == '\n')
				break;
		}else if(rc == 0)
		{
			*ptr = 0;
			return n - 1;
		}
		else 
			return -1;
	}
	*ptr = 0;
	return n;
}
int Fputs(const char * s, FILE * stream){
	int n;
	if((n = fputs(s, stream)) == EOF)
	{
		printf("fputs error");
		exit(1);
	}
	return n;


}
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval * timeout){
	int n;
	if( (n = select(nfds, readfds, writefds, exceptfds,timeout)) == -1)
		perr_exit("select error");
	return n;
}
int max(int a, int b)
{
	return a > b ? a:b;
}
int Poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int n;
	n = poll(fds, nfds, timeout);
	if(n == -1)
		perr_exit("poll error:");
	return n;
}
int Epoll_create(int size)
{
	int n;
	n = epoll_create(size);
	if(n == -1)
		perr_exit("epoll_create error");
	return n;
}
int Epoll_ctl(int epfd, int op, int fd, struct epoll_event * event)
{
	int n;
	n = epoll_ctl(epfd, op, fd, event);
	if(n == -1)
		perr_exit("epoll_ctl error");
	return n;
}
int Epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout)
{
	int n;
	n = epoll_wait(epfd, events, maxevents, timeout);
	if(n == -1)
		perr_exit("epoll_wait_error");
	return n;
}
