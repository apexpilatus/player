ifdef WEB_PATH

ifndef MUSIC_PATH
$(error "MUSIC_PATH")
endif

ifndef LISTEN_PORT
$(error "LISTEN_PORT")
endif

PLAYER_DEFINES := -D music_path=\"${MUSIC_PATH}\"
PLAYER_DEFINES += -D play_pid_path=\"/run/web_play_pid\"
PLAYER_DEFINES += -D listen_port=${LISTEN_PORT}
PLAYER_DEFINES += -D streamer_port=8888
PLAYER_DEFINES += -D android_client_port=9696
PLAYER_DEFINES += -D streamer_host=\"streamer\"
PLAYER_DEFINES += -D web_select=\"$(WEB_PATH)/web_select\"
PLAYER_DEFINES += -D html_main=\"$(WEB_PATH)/html_main\"
PLAYER_DEFINES += -D html_tracks=\"$(WEB_PATH)/html_tracks\"
PLAYER_DEFINES += -D html_cd_control=\"$(WEB_PATH)/html_cd_control\"
PLAYER_DEFINES += -D html_albums=\"$(WEB_PATH)/html_albums\"
PLAYER_DEFINES += -D resp_err=\"$(WEB_PATH)/resp_err\"
PLAYER_DEFINES += -D data_static=\"$(WEB_PATH)/data_static\"
PLAYER_DEFINES += -D data_picture=\"$(WEB_PATH)/data_picture\"
PLAYER_DEFINES += -D data_cd=\"$(WEB_PATH)/data_cd\"
PLAYER_DEFINES += -D data_flac_extracted=\"$(WEB_PATH)/data_flac_extracted\"
PLAYER_DEFINES += -D system_poweroff=\"$(WEB_PATH)/system_poweroff\"
PLAYER_DEFINES += -D forward_play_request=\"$(WEB_PATH)/forward_play_request\"
CC := gcc -O2 -Wall

install: web
	rm -f $(WEB_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(WEB_PATH)

web : Makefile src/web.c web_select html_main html_tracks html_cd_control html_albums resp_err data_static data_picture data_cd data_flac_extracted forward_play_request system_poweroff
	$(CC) src/web.c -o web $(PLAYER_DEFINES)

web_select : Makefile src/web_select.c
	$(CC) src/web_select.c -o web_select $(PLAYER_DEFINES)

html_main : Makefile src/html_main.c
	$(CC) src/html_main.c -o html_main $(PLAYER_DEFINES) -lcdda_interface

html_tracks : Makefile src/html_tracks.c
	$(CC) src/html_tracks.c -o html_tracks $(PLAYER_DEFINES) -lFLAC

html_cd_control : Makefile src/html_cd_control.c
	$(CC) src/html_cd_control.c -o html_cd_control $(PLAYER_DEFINES) -lcdda_interface

html_albums : Makefile src/html_albums.c
	$(CC) src/html_albums.c -o html_albums $(PLAYER_DEFINES)

resp_err : src/resp_err.c
	$(CC) src/resp_err.c -o resp_err

data_static : Makefile src/data_static.c static/style_main.css static/script_main.js static/style_tracks.css static/script_tracks.js static/style_cd_control.css static/script_cd_control.js static/style_albums.css static/script_albums.js static/favicon.ico static/favicon152.png static/favicon180.png
	xxd -i static/style_main.css style_main_css.c
	xxd -i static/script_main.js script_main_js.c
	xxd -i static/style_tracks.css style_tracks_css.c
	xxd -i static/script_tracks.js script_tracks_js.c
	xxd -i static/style_cd_control.css style_cd_control_css.c
	xxd -i static/script_cd_control.js script_cd_control_js.c
	xxd -i static/style_albums.css style_albums_css.c
	xxd -i static/script_albums.js script_albums_js.c
	xxd -i static/favicon.ico favicon.c
	xxd -i static/favicon152.png favicon152.c
	xxd -i static/favicon180.png favicon180.c
	$(CC) src/data_static.c style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c style_cd_control_css.c script_cd_control_js.c style_albums_css.c script_albums_js.c favicon.c favicon152.c favicon180.c -o data_static $(PLAYER_DEFINES)
	rm *.c

data_picture : Makefile src/data_picture.c
	$(CC) src/data_picture.c -o data_picture $(PLAYER_DEFINES) -lFLAC

data_flac_extracted : Makefile src/data_flac_extracted.c
	$(CC) src/data_flac_extracted.c -o data_flac_extracted $(PLAYER_DEFINES) -lFLAC

data_cd : Makefile src/data_cd.c src/lib_read_cd.c src/lib_read_cd.h
	$(CC) src/data_cd.c src/lib_read_cd.c -o data_cd $(PLAYER_DEFINES) -lcdda_interface -lcdda_paranoia

forward_play_request : Makefile src/forward_play_request.c
	$(CC) src/forward_play_request.c -o forward_play_request $(PLAYER_DEFINES) -lFLAC

system_poweroff : Makefile src/system_poweroff.c
	$(CC) src/system_poweroff.c -o system_poweroff $(PLAYER_DEFINES)

endif

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i src/*
