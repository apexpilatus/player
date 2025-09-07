STREAMER_PATH := /home/streamer
STREAMER_DEFINES := -D play_pid_path=\"/run/streamer_play_pid\"
STREAMER_DEFINES += -D mix_pid_path=\"/run/streamer_mixer_pid\"
STREAMER_DEFINES += -D listen_port=8888
STREAMER_DEFINES += -D web_select=\"$(STREAMER_PATH)/streamer_select\"
STREAMER_DEFINES += -D html_volume=\"$(STREAMER_PATH)/html_volume\"
STREAMER_DEFINES += -D resp_err=\"$(STREAMER_PATH)/resp_err\"
STREAMER_DEFINES += -D data_static=\"$(STREAMER_PATH)/data_static\"
STREAMER_DEFINES += -D system_poweroff=\"$(STREAMER_PATH)/system_poweroff\"
STREAMER_DEFINES += -D system_play=\"$(STREAMER_PATH)/system_play\"
STREAMER_DEFINES += -D system_volume=\"$(STREAMER_PATH)/system_volume\"
ifdef init
STREAMER_DEFINES += -D PLAYER_AS_INIT
endif
CC := gcc -O2

streamer : Makefile src/web.c streamer_select html_volume resp_err data_static system_play system_volume system_poweroff
	$(CC) src/web.c -o streamer $(STREAMER_DEFINES)

streamer_select : Makefile src/streamer_select.c
	$(CC) src/streamer_select.c -o streamer_select $(STREAMER_DEFINES)

html_volume : Makefile src/html_volume.c
	$(CC) src/html_volume.c -o html_volume $(STREAMER_DEFINES) -lasound

resp_err : src/resp_err.c
	$(CC) src/resp_err.c -o resp_err

data_static : Makefile src/data_static.c static/style_volume.css static/script_volume.js static/favicon.ico
	xxd -i static/style_volume.css style_volume_css.c
	xxd -i static/script_volume.js script_volume_js.c
	xxd -i static/favicon.ico favicon.c
	$(CC) src/data_static.c style_volume_css.c script_volume_js.c favicon.c -o data_static $(STREAMER_DEFINES)
	rm *.c

system_play : Makefile src/system_play.c
	$(CC) src/system_play.c -o system_play $(STREAMER_DEFINES) -lasound

system_volume : Makefile src/system_volume.c
	$(CC) src/system_volume.c -o system_volume $(STREAMER_DEFINES) -lasound

system_poweroff : Makefile src/system_poweroff.c
	$(CC) src/system_poweroff.c -o system_poweroff

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i src/*

.PHONY: install
install:
	rm -f $(STREAMER_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(STREAMER_PATH)
