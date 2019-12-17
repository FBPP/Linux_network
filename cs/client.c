#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>


#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

int main(){
	int lfd;
	lfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0 ,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
	connect(lfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));

	char buf[BUFSIZ];
	int n;
	while(1)
	{
		fgets(buf, sizeof(buf), stdin); //hello world --fgets()--> hello world\n\0
		write(lfd, buf, strlen(buf));
		n = read(lfd, buf, sizeof(buf));
		write(STDOUT_FILENO, buf, n);
	}
	close(lfd);

	return 0;
}
