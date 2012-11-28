#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<glib.h>
#include<regex.h>
#include "queue.h"
#define PAGEMAX 500000
#define FILENAMEMAX 100
char page_buff[PAGEMAX]; 

/* The generate_file_name function strip out invalied characters from string pointed by hostname and copies the remaining to string pointed by filename at most size bytes including terminating null byte ('\0'). */
char * generate_file_name(const char *url, char *filename, size_t size) {
	regex_t regex;
	regmatch_t regmatchs[1];
	int res;
	res = regcomp(&regex, "[\\?/:*|<>\"]", 0);
	if(res) {
		fprintf(stderr, "Could not complie regex\n");
	}
	strncpy(filename, url, size);
	int length = strlen(url);
	int last = (length > size - 1) ? size - 1 : length;
	filename[last] = '\0';
	while(!(res = regexec(&regex, filename, 1, regmatchs, 0))) {
		filename[regmatchs[0].rm_so] = '_';
	}
	regfree(&regex);
	return filename;
}

/* Make sure hostname and path have enough size */
void get_hostname_path_by_url(const char *url, char *hostname, char *path) {
	char *s = strstr(url, "/");
	if(s) {
		int pos = s - url;
		strncpy(hostname, url, pos);
		hostname[pos] = '\0';
		strcpy(path, s);
	}
	else {
		strcpy(hostname, url);
		strcpy(path, "/\0");
	}
}

/* huge work here */
int parse_web_page(queue *queue_ptr, GHashTable *hashtable_ptr, char *src) {
	
}

/* The function get_web_page() get the web page indicated by hostname and copies to the buffer pointed by page_buff at most size bytes. Warning: If there is no null byte among the first size bytes of webpage get, the string in dest will not be null-terminated. The function returns the actual number of saved bytes in dest, or negative value if error occured. */
int get_web_page(char *hostname, char* path, char *dest, size_t size) {
	int sockfd, s, j;
	char buffer[1024];
	char sendBuff[200];
	struct sockaddr_in server_addr;
	struct hostent *host;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	ssize_t nread;
	char *service = "http";
	int nbytes;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	 /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;		 /* Any protocol */
	s = getaddrinfo(hostname, service, &hints, &result);
	if(s != 0) {
		fprintf(stderr, "getaddrinf: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}
	for(rp = result; rp != NULL; rp = rp->ai_next) {
		sockfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if(sockfd == -1)
			continue;
		if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;
		
		close(sockfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		return -1;
	}

	freeaddrinfo(result);

	/*
	 * if((host = (struct hostent*)gethostbyname(hostname)) == NULL) {
		fprintf(stderr,"Gethostname error\n");
		return -1; * get host name error *
	}
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
		return -2; * socket error *
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
	if(connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Connection Error:%s\a\n", strerror(errno));
		return -3; * connection error *
	}
	*/
	sprintf(sendBuff, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, hostname);
	if(send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
		fprintf(stderr, "Send Error:%s\a\n", strerror(errno));
		return -2; /*sending error */
	}
	int cursor = 0;
	while(1) {
		nbytes = recv(sockfd, buffer, 1024, 0);
		if(nbytes == -1) {
			fprintf(stderr,"Read Error: %s\n", strerror(errno));
			break;
		}
		if(nbytes == 0) {
			fprintf(stderr,"Read Status: %s\n", strerror(errno));
			break;
		}
		/* why needed? losing data?
		if(nbytes < 1024)
			buffer[nbytes] = '\0';
		*/
		if(cursor + nbytes > size) {
			nbytes = size - cursor;
			fprintf(stderr, "Rest page cut:%s\n", hostname);
			break;
		}
		strncpy(dest+cursor, buffer, nbytes);
		cursor += nbytes;
	}
	close(sockfd);
	return cursor;
}

/* Start Point */
int main(int argc, char *argv[]) {
	int fd;
	queue *queue_ptr;
	GHashTable *hashtable_ptr;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s URL(without protocol)\a\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}
	queue_ptr = queue_init();
	queue_add_last(queue_ptr, argv[1]);
       	hashtable_ptr = g_hash_table_new(g_str_hash, g_str_equal);
	while(queue_ptr->size > 0) {
		int url_len = queue_get_first_size(queue_ptr);
		char url[url_len + 1];
		char hostname[url_len + 1];
		char path[url_len + 1];
		/* let the function add null terminator */
		queue_remove_first(queue_ptr, url, url_len + 1);
		get_hostname_path_by_url(url, hostname, path);
		int page_size = get_web_page(hostname, path, page_buff, PAGEMAX);
		if(page_size < 0) break;
		page_buff[page_size] = '\0';
		char filename[FILENAMEMAX];
		generate_file_name(url, filename, FILENAMEMAX);
		if((fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
			fprintf(stderr, "Open Error:%s\n", strerror(errno));
			continue;
		}
		if(write(fd, page_buff, page_size + 1) == -1) {
			fprintf(stderr, "Write Error:%s\n", strerror(errno));
			close(fd);
			continue;
		}
		parse_web_page(queue_ptr, hashtable_ptr, page_buff);
	}
	/* if(fd != -1) close(fd); */
	queue_destroy(queue_ptr);
	g_hash_table_destroy(hashtable_ptr);
	exit(EXIT_SUCCESS);
}
