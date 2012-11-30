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

/* Scan every link in the webpage*/
void parse_web_page(queue *q_waiting_ptr, GHashTable *h_waiting_ptr, GHashTable *h_visited_ptr, const char *src) {
	regex_t regex;
	regmatch_t regmatchs[1];
	int cursor = 0;
	int errcode = regcomp(&regex, "(http|HTTP)://[a-zA-Z0-9]+(\\.[a-zA-Z0-9+,;/?&%$#=~_-]+)+", REG_EXTENDED);
	if(errcode) {
		char errbuf[100];
		regerror(errcode, &regex, errbuf, 100);
		fprintf(stderr, "Could not complie regex: %s\n", errbuf);
		return;
	}
	while(!(errcode = regexec(&regex, src+cursor, 1, regmatchs, 0))) {
		/* Throw the http:// part */
		int length = regmatchs[0].rm_eo - (regmatchs[0].rm_so + 7);
		char *url = (char *)malloc(length + 1);
		strncpy(url, src+cursor + regmatchs[0].rm_so + 7, length);
		url[length] = '\0';
		if(g_hash_table_lookup(h_visited_ptr, url) == NULL &&
			g_hash_table_lookup(h_waiting_ptr, url) == NULL) {
			queue_add_last(q_waiting_ptr, url);
			g_hash_table_add(h_waiting_ptr, url);
			fprintf(stderr, "Add [%s]\n", url);
		}
		else {
			free(url);
		}
		cursor += regmatchs[0].rm_eo;
	}
	regfree(&regex);
	return;
}

/* The function get_web_page() get the web page indicated by hostname and copies to the buffer pointed by page_buff at most size bytes. Warning: If there is no null byte among the first size bytes of webpage get, the string in dest will not be null-terminated. The function returns the actual number of saved bytes in dest, or negative value if error occured. */
int get_web_page(const char *hostname, const char* path, char *dest, size_t size) {
	int sockfd, s, j;
	char buffer[1024];
	struct sockaddr_in server_addr;
	struct hostent *host;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	ssize_t nread;
	char *send_buff, *service = "http";
	char *http_request = "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n";
	int nbytes;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	 /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;		 /* Any protocol */
	s = getaddrinfo(hostname, service, &hints, &result);
	if(s != 0) {
		fprintf(stderr, "Could not resolve [%s]: %s\n", hostname, gai_strerror(s));
		return -1;
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
		return -2;
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
	send_buff = (char *)malloc(strlen(http_request) + strlen(hostname) + strlen(path) + 1);
	sprintf(send_buff, http_request, path, hostname);
	if(send(sockfd, send_buff, strlen(send_buff), 0) < 0) {
		fprintf(stderr, "Send Error:%s\a\n", strerror(errno));
		free(send_buff);
		return -2; /*sending error */
	}
	free(send_buff);
	int cursor = 0;
	while(1) {
		nbytes = recv(sockfd, buffer, 1024, 0);
		if(nbytes == -1) {
			fprintf(stderr,"Read Error: %s\n", strerror(errno));
			break;
		}
		if(nbytes == 0) {
			fprintf(stderr,"Read Status [%s%s]: %s\n", hostname, path, strerror(errno));
			break;
		}
		/* why needed? losing data?
		if(nbytes < 1024)
			buffer[nbytes] = '\0';
		*/
		if(cursor + nbytes > size) {
			nbytes = size - cursor;
			fprintf(stderr, "Rest Cut [%s]\n", hostname);
			break;
		}
		strncpy(dest+cursor, buffer, nbytes);
		cursor += nbytes;
	}
	close(sockfd);
	return cursor;
}

void key_destroy_func(gpointer data) {
	free(data);
}

/* Start Point */
int main(int argc, char *argv[]) {
	int fd, seed_len, seed_len_max = 50;
	char folder[seed_len_max];
	queue *q_waiting_ptr;
	GHashTable *h_waiting_ptr, *h_visited_ptr;
	if(argc != 2) {
		fprintf(stderr, "Usage: %s URL(without protocol)\a\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}
	seed_len = strlen(argv[1]) > seed_len_max-2 ? seed_len_max-2 : strlen(argv[1]);
	strncpy(folder, argv[1], seed_len);
	folder[seed_len] = '\0';
	mkdir(folder, 0744);
	folder[seed_len] = '/';
	folder[seed_len+1] = '\0';
	q_waiting_ptr = queue_init();
	queue_add_last(q_waiting_ptr, argv[1]);
	h_waiting_ptr = g_hash_table_new_full(g_str_hash, g_str_equal, key_destroy_func, NULL);
	char *seed = (char *)malloc(strlen(argv[1]) + 1);
	strcpy(seed, argv[1]);
	g_hash_table_add(h_waiting_ptr, seed);
       	h_visited_ptr = g_hash_table_new_full(g_str_hash, g_str_equal, key_destroy_func, NULL);
	while(q_waiting_ptr->size > 0) {
		int url_len = queue_get_first_size(q_waiting_ptr);
		char *url = (char *)malloc(url_len + 1);
		char hostname[url_len + 1];
		char path[url_len + 1];
		/* let the function add null terminator */
		queue_remove_first(q_waiting_ptr, url, url_len + 1);
		g_hash_table_remove(h_waiting_ptr, url);
		get_hostname_path_by_url(url, hostname, path);
		int page_size = get_web_page(hostname, path, page_buff, PAGEMAX);
		if(page_size < 0) continue;
		page_buff[page_size] = '\0';
		int folder_len = strlen(folder);
		char filename[FILENAMEMAX+folder_len];
		strncpy(filename, folder, folder_len);
		generate_file_name(url, filename+folder_len, FILENAMEMAX);

		if((fd = open(filename, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
			fprintf(stderr, "Open Error:%s\n", strerror(errno));
			continue;
		}
		if(write(fd, page_buff, page_size + 1) == -1) {
			fprintf(stderr, "Write Error:%s\n", strerror(errno));
			close(fd);
			continue;
		}
		g_hash_table_add(h_visited_ptr, url);
		parse_web_page(q_waiting_ptr, h_waiting_ptr, h_visited_ptr, page_buff);
		fprintf(stderr, "Waiting:[%d]\tWaiting_h:[%d]\tVisited:[%d]\n", q_waiting_ptr->size, g_hash_table_size(h_waiting_ptr), g_hash_table_size(h_visited_ptr));
	}
	/* if(fd != -1) close(fd); */
	queue_destroy(q_waiting_ptr);
	g_hash_table_destroy(h_waiting_ptr);
	g_hash_table_destroy(h_visited_ptr);
	exit(EXIT_SUCCESS);
}
