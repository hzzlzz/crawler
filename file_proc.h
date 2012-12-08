#include <stdlib.h>

/* The generate_file_name function strip out invalied characters from string pointed by hostname and copies the remaining to string pointed by filename at most size bytes including terminating null byte ('\0'). */
char * generate_file_name(const char *, char *, size_t);

/* The function will turn url to reasonable filename, get rid of http header from webpage and save the webpage under the filename into directory indicated by folder */
int save_webpage_to_file(const char *, const char *, const char *, size_t);
