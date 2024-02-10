WEB_PATH := /home/player
MUSIC_PATH := /home/store/music
WEB_DEFINES := -D music=\"$(MUSIC_PATH)\"
WEB_DEFINES += -D resp_main=\"$(WEB_PATH)/resp_main\"
WEB_DEFINES += -D data_static=\"$(WEB_PATH)/data_static\"
WEB_DEFINES += -D data_picture=\"$(WEB_PATH)/data_picture\"
WEB_DEFINES += -D resp_err=\"$(WEB_PATH)/resp_err\"
WEB_DEFINES += -D system_poweroff=\"$(WEB_PATH)/system_poweroff\"
ifdef init
WEB_DEFINES += -D WEB_INIT
endif
CC := clang -O2

web : web.c resp_main data_static data_picture resp_err system_poweroff
	clang-format -i web.c
	$(CC) web.c -o web $(WEB_DEFINES)

resp_main : resp_main.c
	clang-format -i resp_main.c
	$(CC) resp_main.c -o resp_main $(WEB_DEFINES)

data_static : data_static.c
	xxd -i style.css style_css.c
	clang-format -i data_static.c
	$(CC) data_static.c style_css.c -o data_static
	rm style_css.c

data_picture : data_picture.c
	clang-format -i data_picture.c
	$(CC) data_picture.c -o data_picture $(WEB_DEFINES) -lFLAC

resp_err : resp_err.c
	clang-format -i resp_err.c
	$(CC) resp_err.c -o resp_err

system_poweroff : system_poweroff.c
	clang-format -i system_poweroff.c
	$(CC) system_poweroff.c -o system_poweroff $(WEB_DEFINES)
