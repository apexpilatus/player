PLAYER_PATH := /home/player
MUSIC_PATH := /home/music
PLAYER_DEFINES := -D music_path=\"$(MUSIC_PATH)\"
PLAYER_DEFINES += -D html_tracks=\"$(PLAYER_PATH)/html_tracks\"
PLAYER_DEFINES += -D html_albums=\"$(PLAYER_PATH)/html_albums\"
PLAYER_DEFINES += -D resp_err=\"$(PLAYER_PATH)/resp_err\"
PLAYER_DEFINES += -D data_static=\"$(PLAYER_PATH)/data_static\"
PLAYER_DEFINES += -D data_picture=\"$(PLAYER_PATH)/data_picture\"
PLAYER_DEFINES += -D system_poweroff=\"$(PLAYER_PATH)/system_poweroff\"
PLAYER_DEFINES += -D system_play_flac=\"$(PLAYER_PATH)/system_play_flac\"
PLAYER_DEFINES += -D system_volume=\"$(PLAYER_PATH)/system_volume\"
PLAYER_DEFINES += -D system_setdate=\"$(PLAYER_PATH)/system_setdate\"
ifdef init
PLAYER_DEFINES += -D PLAYER_AS_INIT
endif
CC := gcc -O2

web : Makefile src/web.c html_tracks html_albums resp_err data_static data_picture system_play_flac system_volume system_poweroff system_setdate
	$(CC) src/web.c -o web $(PLAYER_DEFINES)

html_tracks : Makefile src/html_tracks.c
	$(CC) src/html_tracks.c -o html_tracks $(PLAYER_DEFINES) -lFLAC

html_albums : Makefile src/html_albums.c
	$(CC) src/html_albums.c -o html_albums $(PLAYER_DEFINES)

resp_err : src/resp_err.c
	$(CC) src/resp_err.c -o resp_err

data_static : Makefile src/data_static.c static/style_main.css static/script_main.js static/style_tracks.css static/script_tracks.js static/style_albums.css static/script_albums.js static/index.html static/favicon.ico static/favicon152.png static/favicon180.png
	xxd -i static/style_main.css style_main_css.c
	xxd -i static/script_main.js script_main_js.c
	xxd -i static/style_tracks.css style_tracks_css.c
	xxd -i static/script_tracks.js script_tracks_js.c
	xxd -i static/style_albums.css style_albums_css.c
	xxd -i static/script_albums.js script_albums_js.c
	xxd -i static/index.html index_html.c
	xxd -i static/favicon.ico favicon.c
	xxd -i static/favicon152.png favicon152.c
	xxd -i static/favicon180.png favicon180.c
	$(CC) src/data_static.c style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c style_albums_css.c script_albums_js.c index_html.c favicon.c favicon152.c favicon180.c -o data_static $(PLAYER_DEFINES)
	rm style_main_css.c
	rm script_main_js.c
	rm style_tracks_css.c
	rm script_tracks_js.c
	rm style_albums_css.c
	rm script_albums_js.c
	rm index_html.c
	rm favicon.c
	rm favicon152.c
	rm favicon180.c

data_picture : Makefile src/data_picture.c
	$(CC) src/data_picture.c -o data_picture $(PLAYER_DEFINES) -lFLAC

system_play_flac : Makefile src/system_play_flac.c
	$(CC) src/system_play_flac.c -o system_play_flac $(PLAYER_DEFINES) -lasound -lFLAC

system_volume : Makefile src/system_volume.c
	$(CC) src/system_volume.c -o system_volume $(PLAYER_DEFINES) -lasound

system_poweroff : Makefile src/system_poweroff.c
	$(CC) src/system_poweroff.c -o system_poweroff $(PLAYER_DEFINES)

system_setdate : Makefile src/system_setdate.c
	$(CC) src/system_setdate.c -o system_setdate $(PLAYER_DEFINES)

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i src/*.c

.PHONY: install
install:
	rm -f $(PLAYER_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(PLAYER_PATH)