#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct list_node
{
	char *data;
	node_pointer next;
};

queue * queue_init() {
	queue *queue_ptr = (queue *)malloc(sizeof(queue));
	queue_ptr->head = (node_pointer)malloc(sizeof(struct list_node));
	if(queue_ptr->head == NULL) return NULL;
	queue_ptr->tail = (node_pointer)malloc(sizeof(struct list_node));
	if(queue_ptr->tail == NULL) {
		free(queue_ptr->head);
		return NULL;
	}
	queue_ptr->tail->next = NULL;
	queue_ptr->head->next = queue_ptr->tail;
	queue_ptr->size = 0;
	return queue_ptr;
}

void queue_add_last(queue *queue_ptr, const char *data) {
	node_pointer new_tail;
	queue_ptr->tail->data = strdup(data);
	if(queue_ptr->tail->data == NULL)
		handle_error("strdup data");
	new_tail = (node_pointer)malloc(sizeof(struct list_node));
	if(new_tail == NULL)
		handle_error("malloc tail");
	new_tail->next = NULL;
	queue_ptr->tail->next = new_tail;
	queue_ptr->tail = new_tail;
	queue_ptr->size++;
}

int queue_get_first_size(queue *queue_ptr) {
	node_pointer node_ptr = queue_ptr->head->next;
	if(node_ptr->next == NULL)
		return -1;
	return strlen(node_ptr->data);
}

char * queue_remove_first(queue *queue_ptr, char *data, size_t size) {
	node_pointer node_ptr;
	if(queue_ptr->head->next->next == NULL) {
		return NULL;
	}
	node_ptr = queue_ptr->head->next;
	strncpy(data, node_ptr->data, size);
	queue_ptr->head->next = node_ptr->next;
	free(node_ptr->data);
	free(node_ptr);
	queue_ptr->size--;
	return data;
}

bool queue_contains(queue *queue_ptr, char *data) {
	node_pointer current = queue_ptr->head->next;
	while(current->next != NULL) {
		if(strcmp(current->data, data) == 0) {
			return true;
		}
		current = current->next;
	}
	return false;
}

void queue_clear(queue *queue_ptr) {
	node_pointer current = queue_ptr->head->next;
	while(current->next != NULL) {
		node_pointer next = current->next;
		free(current->data);
		free(current);
		current = next;
		queue_ptr->size--;
	}
	queue_ptr->head->next = current;
}

void queue_destroy(queue *queue_ptr) {
	node_pointer current = queue_ptr->head->next;
	free(queue_ptr->head);
	while(current->next != NULL) {
		node_pointer next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	free(current);
	free(queue_ptr);
}
