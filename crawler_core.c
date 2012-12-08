#include "crawler_core.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <glib.h>

#include "queue.h"
#include "http_parser.h"
#include "http_requester.h"
#include "file_proc.h"

#define PAGEMAX 500000

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

/* Add one url to the waiting list */
struct thread_info {
	pthread_t thread_id;
	queue *q_waiting_ptr;
	GHashTable *h_waiting_ptr;
	GHashTable *h_visited_ptr;
	char *folder;
	size_t max_page_size;
	size_t max_waiting;
	size_t max_total;
	pthread_mutex_t *mutex;
};

/* Function provided to g_hash_table_init to free the memory allocated to key when destroying table or removing from table */
void key_destroy_func(gpointer data) {
	free(data);
}

/* Start to crawl the web according to the parameter given */
static void * crawling(void *arg) {

	int url_len, page_size, sleep_times;
	size_t max_page_size, max_waiting, max_total;
	char *folder, *url, *hostname, *path, *page_buff;
	queue *q_waiting_ptr;
	GHashTable *h_waiting_ptr, *h_visited_ptr;
	pthread_mutex_t *mutex;

	struct thread_info *tinfo = (struct thread_info *) arg;

	q_waiting_ptr = tinfo->q_waiting_ptr;
	h_waiting_ptr = tinfo->h_waiting_ptr;
	h_visited_ptr = tinfo->h_visited_ptr;
	folder = tinfo->folder;
	max_page_size = tinfo->max_page_size;
	max_waiting = tinfo->max_waiting;
	max_total = tinfo->max_total;
	mutex = tinfo->mutex;
	
	page_buff = (char *)malloc(max_page_size);
	if(page_buff == NULL) handle_error("malloc page_buff");

	sleep_times = 0;
	while(1) {
		/* If there is no url, wait 5 seconds,
		 * no more than 2 times */
		/* Notice: there are better way to handle this
		 * Place of improvement */
		if(q_waiting_ptr->size == 0) {
			if(sleep_times < 2) {
				sleep_times++;
				sleep(1);
				continue;
			}
			else {
				break;
			}
		}
		/* freeze to get the lock */
		if(pthread_mutex_lock(mutex) != 0)
			handle_error("pthread_mutex_lock");
		if(q_waiting_ptr->size == 0) {
			if(pthread_mutex_unlock(mutex) != 0)
				handle_error("pthread_mutex_unlock");
			continue;
		}
		if(g_hash_table_size(h_visited_ptr) >= max_total) {
			if(pthread_mutex_unlock(mutex) != 0)
				handle_error("pthread_mutex_unlock");
			/* Break situation */
			break;
		}
		/* Get length of next url to allocate memory */
		url_len = queue_get_first_size(q_waiting_ptr);
		url = (char *)malloc(url_len + 1);
		if(url == NULL) handle_error("malloc url");

		queue_remove_first(q_waiting_ptr, url, url_len + 1);
		g_hash_table_remove(h_waiting_ptr, url);

		/* Mark the url visited
		 * (despite the result of accessing),
		 * considering for thread */
		g_hash_table_add(h_visited_ptr, url);
		
		if(pthread_mutex_unlock(mutex) != 0)
			handle_error("pthread_mutex_unlock");

		hostname = (char *)malloc(url_len + 1);
		if(hostname == NULL) handle_error("malloc hostname");
		path = (char *)malloc(url_len + 1);
		if(hostname == NULL) handle_error("malloc path");
		get_hostname_path_by_url(url, hostname, path);

		/* they should not share page_buff */
		page_size = get_web_page(hostname, path,
				page_buff, max_page_size);

		free(hostname);
		free(path);

		if(page_size < 0) continue; /* get failure */

		if(save_webpage_to_file(folder, url,
					page_buff, page_size) < 0)
			continue; /* save failure */

		/* lock could move into the function for
		 * performance matter */
		if(pthread_mutex_lock(mutex) != 0)
			handle_error("pthread_mutex_lock");

		/* Extract all urls from the page and save them in 
		 * the waiting list */
		parse_web_page(q_waiting_ptr, h_waiting_ptr,
				h_visited_ptr, page_buff, max_waiting);

		if(pthread_mutex_unlock(mutex) != 0)
			handle_error("pthread_mutex_unlock");

		fprintf(stderr, "Waiting:[%d]\tVisited:[%d]\n", 
				q_waiting_ptr->size,
				g_hash_table_size(h_visited_ptr));
	}
	
	free(page_buff);

	return 0;
}

/* Function for crawling the web starting at seed. Notice: GHashTable will free the memory allocated to the key, and only be thread safe after g_thread_init() */
int start(const char *seed, size_t max_waiting,
		size_t max_total, size_t num_threads) {

	int seed_len, tnum, s;
	char *folder, *init_url;
	queue *q_waiting_ptr;
	GHashTable *h_waiting_ptr, *h_visited_ptr;
	struct thread_info *tinfo;
	/* statically allocated, no error checking here */
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	if(mkdir(seed, 0744) == -1) handle_error_en(errno, "mkdir");

	seed_len = strlen(seed);
	folder = (char *)malloc(seed_len+2);
	if(folder == NULL) handle_error("malloc folder");

	strcpy(folder, seed);
	folder[seed_len] = '/';
	folder[seed_len+1] = '\0';

	/* Queue of url waiting to be visited */
	q_waiting_ptr = queue_init();
	/* Hashtable of url waiting to be visited, for fast querying */
	h_waiting_ptr = g_hash_table_new_full(
			g_str_hash, g_str_equal,
			key_destroy_func, NULL);
	/* Hashtable of visited list */
	h_visited_ptr = g_hash_table_new_full(
			g_str_hash, g_str_equal,
			key_destroy_func, NULL);

	init_url = strdup(seed);
	if(init_url == NULL) handle_error("strdup seed");

	queue_add_last(q_waiting_ptr, init_url);
	g_hash_table_add(h_waiting_ptr, init_url);

	tinfo = calloc(num_threads, sizeof(struct thread_info));

	if(tinfo == NULL) handle_error("calloc");

	for(tnum = 0; tnum < num_threads; tnum++) {
		tinfo[tnum].q_waiting_ptr = q_waiting_ptr;
		tinfo[tnum].h_waiting_ptr = h_waiting_ptr;
		tinfo[tnum].h_visited_ptr = h_visited_ptr;
		tinfo[tnum].folder = folder;
		tinfo[tnum].max_page_size = PAGEMAX;
	       	tinfo[tnum].max_waiting = max_waiting;
		tinfo[tnum].max_total = max_total;
		tinfo[tnum].mutex = &mutex;

		s = pthread_create(&tinfo[tnum].thread_id, NULL,
				&crawling, &tinfo[tnum]);

		if(s != 0) handle_error_en(s, "pthread_create");
	}

	for(tnum = 0; tnum < num_threads; tnum++) {
		s = pthread_join(tinfo[tnum].thread_id, NULL);

		if(s != 0) handle_error_en(s, "pthread_join");
		
		/* res not dealt with */
	}

	free(tinfo);
	/* free(seed) not needed, visited hash table will free it */
	free(folder);
	
	queue_destroy(q_waiting_ptr);
	g_hash_table_destroy(h_waiting_ptr);
	g_hash_table_destroy(h_visited_ptr);

	return 0;
}
