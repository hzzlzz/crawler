#ifndef queue_h
#define queue_h

#include <stdbool.h>
#include <stdlib.h>

typedef struct list_node *node_pointer;

typedef struct {
	node_pointer head; /* do not save actual value */
	node_pointer tail; /* do not save actual value */
	int size;
} queue;

queue * queue_init();

void queue_add_last(queue *, const char *);

int queue_get_first_size(queue *);

char * queue_remove_first(queue *, char *, size_t);

bool queue_contains(queue *, char *);

void queue_clear(queue *);

void queue_destroy(queue *);

#endif
