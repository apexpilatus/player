ifdef STREAMER_PATH

CC := gcc -O3 -Wall -march=native
RC := rustc -C debuginfo=0 -C opt-level=3 -C target-cpu=native

install: web
	rm -f $(STREAMER_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(STREAMER_PATH)

web: rust/web.rs rust/err_codes.rs rust/page_home.rs rust/page_tracks.rs rust/player.rs rust/proxy.rs rust/volume.rs html/home.html html/tracks.html js/home.js style/home.css js/tracks.js style/tracks.css play getvolume setvolume
	$(RC) -o streamer rust/web.rs

play: c/play.c
	$(CC) -o play c/play.c -lFLAC -lasound

getvolume: c/getvolume.c
	$(CC) -o getvolume c/getvolume.c -lasound

setvolume: c/setvolume.c
	$(CC) -o setvolume c/setvolume.c -lasound


endif

.PHONY: err
err:
	$(error "STREAMER_PATH")

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i c/*
	rustfmt rust/*
