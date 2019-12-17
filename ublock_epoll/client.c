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
#define MAXLINE 10

int main(){
	int lfd;
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	char buf[MAXLINE];
	int i;
	char ch = 'a';

	memset(&serv_addr, 0 ,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
	Connect(lfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
	
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

		Write(lfd, buf, sizeof(buf));
		sleep(3);
	}

	close(lfd);

	return 0;
}
