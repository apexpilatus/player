ifdef STORE_PATH

CC := gcc -O3 -Wall -march=native
RC := rustc -C debuginfo=0 -C opt-level=3 -C target-cpu=native

install: web
	rm -f $(STORE_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(STORE_PATH)

web: rust/web.rs rust/text_meta.rs rust/system_touch.rs rust/page_home.rs rust/page_albums.rs rust/err_codes.rs rust/data_static.rs rust/data_flac.rs rust/data_picture.rs style/albums.css style/home.css pictures/favicon.ico pictures/favicon120.png pictures/favicon180.png js/albums.js js/home.js html/albums.html html/home.html picture meta flac touch
	$(RC) -o store rust/web.rs

picture: c/picture.c
	$(CC) -o picture c/picture.c -lFLAC

meta: c/meta.c
	$(CC) -o meta c/meta.c -lFLAC

flac: c/flac.c
	$(CC) -o flac c/flac.c -lFLAC

touch: c/touch.c
	$(CC) -o touch c/touch.c

endif

.PHONY: err
err:
	$(error "STORE_PATH")

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i c/*
	rustfmt rust/*
