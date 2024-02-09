WEB_PATH := /home/player
MUSIC_PATH := /home/store/music
MUSIC_DEFS := -D music=\"$(MUSIC_PATH)\"
PICTURE_ALB_DEFS := -D page_err=\"$(WEB_PATH)/page_err\"
WEB_DEFINES := -D music=\"$(MUSIC_PATH)\"
WEB_DEFINES += -D page_main=\"$(WEB_PATH)/page_main\"
WEB_DEFINES += -D picture_favicon=\"$(WEB_PATH)/picture_favicon\"
WEB_DEFINES += -D picture_album=\"$(WEB_PATH)/picture_album\"
WEB_DEFINES += -D page_err=\"$(WEB_PATH)/page_err\"
WEB_DEFINES += -D system_poweroff=\"$(WEB_PATH)/system_poweroff\"
ifdef init
WEB_DEFINES += -D WEB_INIT
POWER_DEFS := -D WEB_INIT
endif
CC := clang -O2

web : web.c page_main picture_favicon picture_album page_err system_poweroff
	clang-format -i web.c
	$(CC) web.c -o web $(WEB_DEFINES)

page_main : page_main.c
	clang-format -i page_main.c
	$(CC) page_main.c -o page_main $(MUSIC_DEFS)

picture_favicon : picture_favicon.c
	clang-format -i picture_favicon.c
	$(CC) picture_favicon.c -o picture_favicon

picture_album : picture_album.c
	clang-format -i picture_album.c
	$(CC) picture_album.c -o picture_album $(PICTURE_ALB_DEFS) -lFLAC

page_err : page_err.c
	clang-format -i page_err.c
	$(CC) page_err.c -o page_err

system_poweroff : system_poweroff.c
	clang-format -i system_poweroff.c
	$(CC) system_poweroff.c -o system_poweroff $(POWER_DEFS)
