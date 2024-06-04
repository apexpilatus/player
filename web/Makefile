PLAYER_PATH := /home/player
MUSIC_PATH := /home/music
PLAYER_DEFINES := -D music_path=\"$(MUSIC_PATH)\"
PLAYER_DEFINES += -D html_tracks=\"$(PLAYER_PATH)/html_tracks\"
PLAYER_DEFINES += -D html_albums=\"$(PLAYER_PATH)/html_albums\"
PLAYER_DEFINES += -D resp_err=\"$(PLAYER_PATH)/resp_err\"
PLAYER_DEFINES += -D data_static=\"$(PLAYER_PATH)/data_static\"
PLAYER_DEFINES += -D data_picture=\"$(PLAYER_PATH)/data_picture\"
PLAYER_DEFINES += -D system_poweroff=\"$(PLAYER_PATH)/system_poweroff\"
PLAYER_DEFINES += -D system_play=\"$(PLAYER_PATH)/system_play\"
PLAYER_DEFINES += -D system_volume=\"$(PLAYER_PATH)/system_volume\"
ifdef init
PLAYER_DEFINES += -D PLAYER_AS_INIT
endif
CC := clang -O2

web : Makefile web.c html_tracks html_albums resp_err data_static data_picture system_play system_volume system_poweroff
	clang-format -i web.c
	$(CC) web.c -o web $(PLAYER_DEFINES)

html_tracks : Makefile html_tracks.c
	clang-format -i html_tracks.c
	$(CC) html_tracks.c -o html_tracks $(PLAYER_DEFINES) -lFLAC

html_albums : Makefile html_albums.c
	clang-format -i html_albums.c
	$(CC) html_albums.c -o html_albums $(PLAYER_DEFINES)

resp_err : resp_err.c
	clang-format -i resp_err.c
	$(CC) resp_err.c -o resp_err

data_static : Makefile data_static.c style_main.css script_main.js style_tracks.css script_tracks.js style_albums.css script_albums.js index.html
	xxd -i style_main.css style_main_css.c
	xxd -i script_main.js script_main_js.c
	xxd -i style_tracks.css style_tracks_css.c
	xxd -i script_tracks.js script_tracks_js.c
	xxd -i style_albums.css style_albums_css.c
	xxd -i script_albums.js script_albums_js.c
	xxd -i index.html index_html.c
	clang-format -i data_static.c
	$(CC) data_static.c style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c style_albums_css.c script_albums_js.c index_html.c -o data_static $(PLAYER_DEFINES)
	rm style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c style_albums_css.c script_albums_js.c index_html.c

data_picture : Makefile data_picture.c
	clang-format -i data_picture.c
	$(CC) data_picture.c -o data_picture $(PLAYER_DEFINES) -lFLAC

system_play : Makefile system_play.c
	clang-format -i system_play.c
	$(CC) system_play.c -o system_play $(PLAYER_DEFINES) -lasound -lFLAC

system_volume : Makefile system_volume.c
	clang-format -i system_volume.c
	$(CC) system_volume.c -o system_volume $(PLAYER_DEFINES) -lasound

system_poweroff : Makefile system_poweroff.c
	clang-format -i system_poweroff.c
	$(CC) system_poweroff.c -o system_poweroff $(PLAYER_DEFINES)

.PHONY: clean
clean:
	rm $(shell ls|grep -v "Makefile\|\.")

