#include<glib.h>
#include "linked_list.h"


void hashtable_init(GHashTable *hashtable) {
	hashtable = g_hash_table_new(g_str_hash, g_str_equal);
	return;
}

void add_visited_url(GHashTable *hashtable, const char *url) {
	g_hash_table_insert(hashtable, url, NULL);
	return;
}

bool remove_visited_url(GHashTable *hashtable, const char *url) {
	return g_hash_table_remove(hashtable, url);
}

int get_visited_url_number(GHashTable *hashtable) {
	return g_hash_table_size(hashtable);
}

bool visited_url_contains(GHashTable *hashtable, const char *url) {
	
}
