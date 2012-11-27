#include "linked_list.h"

int linked_list_init(linked_list *list_ptr) { /*do not init twice*/
	list_ptr = (linked_list *)malloc(sizeof(linked_list));
	list_ptr->head = (node_pointer)malloc(sizeof(struct list_node));
	if(list_ptr->head == NULL) return 1;
	list_ptr->tail = (node_pointer)malloc(sizeof(struct list_node));
	if(list_ptr->tail == NULL) {
		free(list_ptr->head);
		return 1;
	}
	list_ptr->tail->next = NULL;
	list_ptr->head->next = list_ptr->tail;
	list_ptr->size = 0;
	return 0;
}

int linked_list_add_last(linked_list *list_ptr, const char *data) {
	if(strlen(data) > DATA_SIZE) return 1;
	node_pointer new_tail = (node_pointer)malloc(sizeof(struct list_node));
	char test[100];
	char *test_o = "www.baidu.com";
	strcpy(test, test_o);
	strncpy(list_ptr->tail->data, data, DATA_SIZE);
	list_ptr->tail->next = new_tail;
	list_ptr->tail = new_tail;
	list_ptr->size++;
	return 0;
}

int linked_list_remove_first(linked_list *list_ptr, char *data, size_t size) {
	if(list_ptr->head->next->next == NULL) {
		return 1;
	}
	node_pointer node_ptr = list_ptr->head->next;
	strncpy(data, node_ptr->data, size);
	list_ptr->head->next = node_ptr->next;
	free(node_ptr);
	list_ptr->size--;
	return 0;
}

bool linked_list_contains(linked_list *list_ptr, char *data) {
	node_pointer current = list_ptr->head->next;
	while(current->next != NULL) {
		if(strcmp(current->data, data) == 0) {
			return true;
		}
		current = current->next;
	}
	return false;
}

int linked_list_destroy(linked_list *list_ptr) {
	node_pointer current = list_ptr->head;
	while(current != NULL) {
		node_pointer next = current->next;
		free(current);
		current = next;
	}
	free(list_ptr);
	return 0;
}
