#include "http_parser.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <regex.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

/* Notice: Make sure hostname and path have enough size */
void get_hostname_path_by_url(const char *url, char *hostname, char *path) {
	char *s;

	s = strstr(url, "/");
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

/* Scan every link in the webpage, save unique links into the waiting list */
void parse_web_page(queue *q_waiting_ptr, GHashTable *h_waiting_ptr,
		GHashTable *h_visited_ptr, const char *src,
		size_t max_waiting) {
	int cursor, errcode, length;
	char errbuf[100], *url;
	regex_t regex;
	regmatch_t regmatchs[1];

	errcode = regcomp(&regex,
			"(http|HTTP)://[a-zA-Z0-9]+(\\.[a-zA-Z0-9+,;/?&%$#=~_-]+)+",
			REG_EXTENDED);
	if(errcode) {
		regerror(errcode, &regex, errbuf, 100);
		handle_error(errbuf);
	}

	cursor = 0;
	while(!(errcode = regexec(&regex, src+cursor, 1, regmatchs, 0))) {
		/* Throw the http:// part */
		length = regmatchs[0].rm_eo - (regmatchs[0].rm_so + 7);
		url = (char *)malloc(length + 1);
		if(url == NULL)
			handle_error("malloc url");

		strncpy(url, src+cursor+regmatchs[0].rm_so+7, length);
		url[length] = '\0';

		if(g_hash_table_lookup(h_visited_ptr, url) == NULL &&
		   g_hash_table_lookup(h_waiting_ptr, url) == NULL) 
		{
			if(q_waiting_ptr->size < max_waiting) {
				queue_add_last(q_waiting_ptr, url);
				g_hash_table_add(h_waiting_ptr, url);
				/*fprintf(stderr, "[%s] Added\n", url);*/
			}
			else {
				free(url);
				break;
			}
		}
		else {
			free(url);
		}
		cursor += regmatchs[0].rm_eo;
	}
	regfree(&regex);
	return;
}
