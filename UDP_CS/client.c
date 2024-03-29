#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define SERV_PORT 8000

int main()
{
	struct sockaddr_in serv_addr;
	int sockfd;
	char buf[BUFSIZ];
	int n;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port = htons(SERV_PORT);

	while(fgets(buf, BUFSIZ, stdin) != NULL)
		{
		
			n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
			if(n == -1)
				perror("sendto error");
			n = recvfrom(sockfd, buf, BUFSIZ, 0, NULL, 0);
			if(n == -1)
				perror("recvfrom error");

			fputs(buf, stdout);

		}
		close(sockfd);
	
	return 0;
}

