ifdef STORE_PATH

CC := gcc -O2 -Wall -march=native
RC := rustc -C debuginfo=0 -C opt-level=3 -C target-cpu=native

install: web
	rm -f $(STORE_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(STORE_PATH)

web: rust/web.rs picture meta flac_extract touch
	$(RC) -o web rust/web.rs

picture: c/picture.c
	$(CC) -o picture c/picture.c -lFLAC

meta: c/meta.c
	$(CC) -o meta c/meta.c -lFLAC

flac_extract: c/flac_extract.c
	$(CC) -o flac_extract c/flac_extract.c -lFLAC

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
