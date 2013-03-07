crawler: main.o crawler_core.o file_proc.o http_parser.o http_requester.o queue.o
	$(CC) -o $@ $^ `pkg-config --libs glib-2.0` -lpthread
.c.o:
	$(CC) -O2 -c $< `pkg-config --cflags glib-2.0` -Wall -pedantic -g

all: crawler
clean: all
	rm -f *.o
