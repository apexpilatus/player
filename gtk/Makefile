ifndef URL
	URL := "http://streamer"
endif

DEFINES := -D URL=\"$(URL)\"

CC := gcc -O2 -Wall

browser : Makefile gtk.c
	$(CC) -o ../../browser $(DEFINES) $(shell pkg-config --cflags webkitgtk-6.0) gtk.c $(shell pkg-config --libs webkitgtk-6.0)
