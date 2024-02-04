WEB_PATH := /home/store/works/player/web
MUSIC_PATH := /home/store/music
WEB_DEFINES := -D page_main=\"$(WEB_PATH)/page_main\"
WEB_DEFINES += -D picture_favicon=\"$(WEB_PATH)/picture_favicon\"
WEB_DEFINES += -D page_err=\"$(WEB_PATH)/page_err\"
CC := clang -O2

web : web.c page_main picture_favicon page_err
	clang-format -i web.c
	$(CC) web.c -o web $(WEB_DEFINES)

page_main : page_main.c
	clang-format -i page_main.c
	$(CC) page_main.c -o page_main -D music=\"$(MUSIC_PATH)\"

picture_favicon : picture_favicon.c
	clang-format -i picture_favicon.c
	$(CC) picture_favicon.c -o picture_favicon

page_err : page_err.c
	clang-format -i page_err.c
	$(CC) page_err.c -o page_err

