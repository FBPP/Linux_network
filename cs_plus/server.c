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

int main(){
	int lfd, cfd;
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in  serv_addr, client_addr;
	bzero(&serv_addr, sizeof(serv_addr));

	socklen_t client_addr_len;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	Listen(lfd, 128);

	client_addr_len = sizeof(client_addr);
	cfd = Accept(lfd,(struct sockaddr *) &client_addr, &client_addr_len);

	char client_IP[BUFSIZ];
	printf("client IP: %s, client port:%d\n",
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr,
			       	client_IP,sizeof(client_IP)),ntohs(client_addr.sin_port));

	char buf[BUFSIZ];
	int n;
	while(1)
	{
		n = Read(cfd, buf, sizeof(buf));

		for(int i = 0; i < n; i++)
			buf[i] = toupper(buf[i]);
		Write(cfd, buf, n);
	}
	Close(lfd);
	Close(cfd);

	return 0;
}
