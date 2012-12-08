#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "crawler_core.h"

/*
void set_nonblocking(int sockfd)
{
	int  opts;
	opts = fcntl(sockfd, F_GETFL);
	if (opts < 0 )
		handle_error("fcntl");
	
	opts  =  opts | O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, opts) < 0 )
		handle_error("fcntl");
}
*/

/* Program starts here */
int main(int argc, char *argv[]) {
	int opt, num_threads, max_waiting, max_total;
	char *seed;

	num_threads = 0;
	max_waiting = 0;
	max_total = 0;
	seed = NULL;

	while((opt = getopt(argc, argv, "t:w:v:")) != -1) {
		switch(opt) {
			case 't':
				num_threads = strtoul(optarg, NULL, 0);
				break;
			case 'w':
				max_waiting = strtoul(optarg, NULL, 0);
				break;
			case 'v':
				max_total = strtoul(optarg, NULL, 0);
				break;
			default:
				fprintf(stderr, "Usage: %s -t num-threads -w waiting-list-size -v max-visit-num init-url(without http://)", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	seed = argv[optind];

	if(num_threads == 0 || max_waiting == 0 || max_total == 0 || seed == NULL) {
		fprintf(stderr, "Usage: %s -t num-threads -w waiting-list-size -v max-visit-num init-url(without http://)\a\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	start(seed, max_waiting, max_total, num_threads);

	exit(EXIT_SUCCESS);
}
