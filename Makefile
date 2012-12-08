crawler: request_page.o queue.o
	$(CC) -o $@ $^ `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lpthread
.c.o:
	$(CC) -c -g $< `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
all: crawler
clean: all
	rm -f *.o
