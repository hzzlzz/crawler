/* http_requester.h -- interface for a simple http requester */
#ifndef _HTTP_REQUESTER_H_
#define _HTTP_REQUESTER_H_

#include <stdlib.h>

/* The function get_web_page() get the web page indicated by hostname and copies to the buffer pointed by page_buff at most size bytes including a terminating null byte. The function returns the actual number of saved bytes in dest, or negative value if error occured. */
int get_web_page(const char *, const char *, char *, size_t);

#endif
