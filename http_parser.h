#include <stdlib.h>
#include <glib.h>

#include "queue.h"

void get_hostname_path_by_url(const char *, char *, char *);

void parse_web_page(queue *, GHashTable *, GHashTable *, const char *, size_t);
