WEB_PATH := /home/store/player
MUSIC_PATH := /home/store/music

CC := clang -O2

web : web.c selector
	clang-format -i web.c
	$(CC) web.c -o web -D selector=\"$(WEB_PATH)/selector\"

selector : selector.c page_main picture_favicon
	clang-format -i selector.c
	$(CC) selector.c -o selector -D page_main=\"$(WEB_PATH)/page_main\" -D picture_favicon=\"$(WEB_PATH)/picture_favicon\"

page_main : page_main.c
	clang-format -i page_main.c
	$(CC) page_main.c -o page_main -D music=\"$(MUSIC_PATH)\"

picture_favicon : picture_favicon.c
	clang-format -i picture_favicon.c
	$(CC) picture_favicon.c -o picture_favicon
