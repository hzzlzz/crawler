#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/errno.h>

int main(int argc, char *argv[]) {
	int sockfd;
	char buffer[1024];
	char sendBuff[200];
	struct sockaddr_in server_addr;
	struct hostent *host;
	int portnumber, nbytes;
	if(argc!=3) {
		fprintf(stderr, "Usage: %s hostname portnumber\a\n", argv[0]);
		exit(1);
	}
	if(((host = gethostbyname(argv[1])) == NULL)) {
		fprintf(stderr,"Gethostname error\n");
		exit(1);
	}
	if((portnumber=atoi(argv[2]))<0)
	{
		fprintf(stderr, "Usage:%s hostname portnumber\a\n", argv[0]);
		exit(1);
	}
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
		exit(1);
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	if(connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Connection Error:%s\a\n", strerror(errno));
		exit(1);
	}
	sprintf(sendBuff, "GET / HTTP/1.0\r\nHost: %s\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 10.0; Windows NT 6.1; Win64; x64; Trident/4.0)\r\n\r\n", argv[1]);
	printf("send buff size: %d\n", strlen(sendBuff));
	if(send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
		fprintf(stderr, "Send Head Error:%s\a\n", strerror(errno));
		exit(1);
	}
	while(1) {
		nbytes = recv(sockfd, buffer, 1024, 0);
		if(nbytes == -1) {
			fprintf(stderr,"Read Error: %s\n", strerror(errno));
			break;
		}
		if(nbytes == 0) {
			fprintf(stderr, "Read Info:%s\n", strerror(errno));
			break;
		}
		if(nbytes < 1024)
			buffer[nbytes] = '\0';
		printf("%s", buffer);
	}
	close(sockfd);
	exit(0);
}
