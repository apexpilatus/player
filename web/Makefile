WEB_PATH := /home/player
MUSIC_PATH := /home/store/music
WEB_DEFINES := -D music=\"$(MUSIC_PATH)\"
WEB_DEFINES += -D resp_main=\"$(WEB_PATH)/resp_main\"
WEB_DEFINES += -D data_static=\"$(WEB_PATH)/data_static\"
WEB_DEFINES += -D data_picture=\"$(WEB_PATH)/data_picture\"
WEB_DEFINES += -D resp_err=\"$(WEB_PATH)/resp_err\"
WEB_DEFINES += -D system_poweroff=\"$(WEB_PATH)/system_poweroff\"
WEB_DEFINES += -D resp_tracks=\"$(WEB_PATH)/resp_tracks\"
ifdef init
WEB_DEFINES += -D WEB_INIT
endif
CC := clang -O2

web : web.c resp_main data_static data_picture resp_err system_poweroff resp_tracks
	clang-format -i web.c
	$(CC) web.c -o web $(WEB_DEFINES)

resp_main : resp_main.c
	clang-format -i resp_main.c
	$(CC) resp_main.c -o resp_main $(WEB_DEFINES)

data_static : data_static.c style_main.css script_main.js style_tracks.css script_tracks.js
	xxd -i style_main.css style_main_css.c
	xxd -i script_main.js script_main_js.c
	xxd -i style_tracks.css style_tracks_css.c
	xxd -i script_tracks.js script_tracks_js.c
	clang-format -i data_static.c
	$(CC) data_static.c style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c -o data_static $(WEB_DEFINES)
	rm style_main_css.c script_main_js.c style_tracks_css.c script_tracks_js.c

data_picture : data_picture.c
	clang-format -i data_picture.c
	$(CC) data_picture.c -o data_picture $(WEB_DEFINES) -lFLAC

resp_err : resp_err.c
	clang-format -i resp_err.c
	$(CC) resp_err.c -o resp_err

system_poweroff : system_poweroff.c
	clang-format -i system_poweroff.c
	$(CC) system_poweroff.c -o system_poweroff $(WEB_DEFINES)

resp_tracks : resp_tracks.c
	clang-format -i resp_tracks.c
	$(CC) resp_tracks.c -o resp_tracks $(WEB_DEFINES)

