#include "http_requester.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define BUFFSIZE 4096

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


/* The function get_web_page() gets the web page indicated by hostname and copies to the buffer pointed by page_buff at most size bytes. The function returns the actual number of saved bytes in dest, or negative value if error occured. */
int get_web_page(const char *hostname, const char* path, char *dest, size_t size) {
	int sockfd, s;
	unsigned char buffer[BUFFSIZE];
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	struct timeval tv_out;
	char *send_buff, *service = "http";
	char *http_request = "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n";
	int nbytes, cursor;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	 /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;		 /* Any protocol */

	s = getaddrinfo(hostname, service, &hints, &result);
	if(s != 0) {
		fprintf(stderr, "Could not resolve [%s]: %s\n",
				hostname, gai_strerror(s));
		return -1;
	}
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family,
				rp->ai_socktype,
				rp->ai_protocol);
		if(sockfd == -1)
			continue;
		if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(sockfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		return -2;
	}

	freeaddrinfo(result);

	send_buff = (char *)malloc(strlen(http_request) + 
			strlen(hostname) + strlen(path) + 1);
	sprintf(send_buff, http_request, path, hostname);

	if(send(sockfd, send_buff, strlen(send_buff), 0) < 0) {
		fprintf(stderr, "Sending Error:%s\a\n",
				strerror(errno));
		free(send_buff);

		return -2; /*sending error */
	}
	free(send_buff);

	/* Set time out of recv, in case dead connection occurs,
	 * causing recv to wait forever. */
	tv_out.tv_sec = 10; /* Max waiting time */
	tv_out.tv_usec = 0;
	s = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
			&tv_out, sizeof(tv_out));
	if(s != 0)
		handle_error_en(errno, "setsockopt");

	cursor = 0;
	fprintf(stderr, "Reading From [%s%s]...\n",
			hostname, path);
	while(1) {
		nbytes = recv(sockfd, buffer, BUFFSIZE, 0);
		if(nbytes == -1) {
			fprintf(stderr,"[%s%s] %s\n",
					hostname, path,
					strerror(errno));
			break;
		}
		if(nbytes == 0) {
			fprintf(stderr,"[%s%s] %s\n",
					hostname, path,
					strerror(errno));
			break;
		}
		if(cursor + nbytes > size) {
			fprintf(stderr, "[%s%s] Rest Cut\n",
					hostname, path);

			nbytes = size - cursor;
			memcpy(dest+cursor, buffer, nbytes);
			cursor += nbytes;
			break;
		}
		memcpy(dest+cursor, buffer, nbytes);
		cursor += nbytes;
	}
	close(sockfd);
	return cursor;
}
