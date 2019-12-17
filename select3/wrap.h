#ifndef _WRAP_H_
#define _WRAP_H_

void perr_exit(const char * s);
int Accept(int fd, struct sockaddr * sa, socklen_t * addrlen);
int Bind(int fd, const struct sockaddr *sa, socklen_t addrlen);
int Connect(int fd, const struct sockaddr * sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int  type, int protocol);
ssize_t Read(int fd, void * ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void * vptr, size_t n);
ssize_t Writen(int fd, const void * vptr, size_t n);
static ssize_t my_read(int fd, char * ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);
int Select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
int max(int, int);
int Fputs(const char*, FILE* );
#endif
