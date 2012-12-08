#include "file_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>

#define FILENAMEMAX 100

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


/* The generate_file_name function strip out invalied characters from string pointed by hostname and copies the remaining to string pointed by filename at most size bytes including terminating null byte ('\0'). */
char * generate_file_name(const char *url, char *filename, size_t size) {
	int res, length, last;
	regex_t regex;
	regmatch_t regmatchs[1];

	res = regcomp(&regex, "[\\?/:*|<>\"]", 0);
	if(res) handle_error_en(errno, "regcomp");

	strncpy(filename, url, size);
	length = strlen(url);
	last = (length > size - 1) ? size - 1 : length;
	filename[last] = '\0';

	while(!(res = regexec(&regex, filename, 1, regmatchs, 0))) {
		filename[regmatchs[0].rm_so] = '_';
	}
	regfree(&regex);

	return filename;
}

/* The function will turn url to reasonable filename, get rid of http header from webpage and save the webpage under the filename into directory indicated by folder */
int save_webpage_to_file(const char *folder, const char *url,
				const char *webpage, size_t page_size) {
	int fd, folder_len, pos;
	char *filename, *s;

	s = strstr(webpage, "\r\n\r\n");
	if(s) {
		pos = s - webpage + 4;
	}
	else {
		pos = 0;
	}

	folder_len = strlen(folder);
	filename = malloc(FILENAMEMAX+folder_len);
	strncpy(filename, folder, folder_len);
	generate_file_name(url, filename+folder_len, FILENAMEMAX);

	if((fd=open(filename, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
		fprintf(stderr, "Open Error:%s\n", strerror(errno));
		free(filename);
		return -1;
	}
	if(write(fd, webpage+pos, page_size-pos) == -1) {
		fprintf(stderr, "Write Error:%s\n", strerror(errno));
		free(filename);
		close(fd);
		return -2;
	}

	free(filename);
	close(fd);
}
