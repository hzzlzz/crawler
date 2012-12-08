crawler: main.o crawler_core.o file_proc.o http_parser.o http_requester.o queue.o
	$(CC) -o $@ $^ `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lpthread
.c.o:
	$(CC) -c $< `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -g -Wall -pedantic

all: crawler
clean: all
	rm -f *.o
