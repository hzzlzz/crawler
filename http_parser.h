/* http_parser.h -- interface for a simple web parser. */
#ifndef _HTTP_PARSER_H_
#define _HTTP_PARSER_H_

#include <stdlib.h>
#include <glib.h>

#include "queue.h"

/* Separates the hostname and path from url */
void get_hostname_path_by_url(const char *, char *, char *);

/* Extracts every url from webpage and save unique ones into list */
void parse_web_page(queue *, GHashTable *, GHashTable *, const char *, size_t);

#endif
