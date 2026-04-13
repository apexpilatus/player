ifdef STREAMER_PATH

ifndef LISTEN_PORT
LISTEN_PORT := 80
endif

ifndef STORE_PORT
STORE_PORT := 80
endif

STREAMER_DEFINES := -D resp_err=\"$(STREAMER_PATH)/resp_err\"
STREAMER_DEFINES += -D listen_port=$(LISTEN_PORT)
STREAMER_DEFINES += -D store_port=$(STORE_PORT)
STREAMER_DEFINES += -D store_host=\"store\"
STREAMER_DEFINES += -D init_script=\"$(STREAMER_PATH)/init.sh\"
STREAMER_DEFINES += -D data_proxy=\"$(STREAMER_PATH)/data_proxy\"
STREAMER_DEFINES += -D select_path=\"$(STREAMER_PATH)/streamer_select\"
STREAMER_DEFINES += -D select_name=\"streamer_select\"
STREAMER_DEFINES += -D sleep_timeout=50000
ifneq ($(DEVICE),proxy)
STREAMER_DEFINES += -D play_pid_path=\"/run/streamer_play_pid\"
STREAMER_DEFINES += -D mix_pid_path=\"/run/streamer_mixer_pid\"
STREAMER_DEFINES += -D html_volume=\"$(STREAMER_PATH)/html_volume\"
STREAMER_DEFINES += -D data_static=\"$(STREAMER_PATH)/data_static\"
STREAMER_DEFINES += -D system_play=\"$(STREAMER_PATH)/system_play\"
STREAMER_DEFINES += -D system_play_bad=\"$(STREAMER_PATH)/system_play_bad\"
STREAMER_DEFINES += -D system_volume=\"$(STREAMER_PATH)/system_volume\"
endif
CC := gcc -O2 -Wall

install: streamer
	rm -f $(STREAMER_PATH)/*
	mv $(shell find . -maxdepth 1 -type f -not -name Makefile) $(STREAMER_PATH)

ifeq ($(DEVICE),proxy)
streamer : Makefile src/web.c data_proxy resp_err streamer_select
	$(CC) src/web.c -o streamer $(STREAMER_DEFINES)
else
streamer : Makefile src/web.c streamer_select html_volume data_proxy resp_err data_static system_play system_play_bad system_volume
	$(CC) src/web.c -o streamer $(STREAMER_DEFINES)
endif

streamer_select : Makefile src/streamer_select.c
	$(CC) src/streamer_select.c -o streamer_select $(STREAMER_DEFINES)

data_proxy : Makefile src/data_proxy.c
	$(CC) src/data_proxy.c -o data_proxy $(STREAMER_DEFINES)

html_volume : Makefile src/html_volume.c
	$(CC) src/html_volume.c -o html_volume $(STREAMER_DEFINES) -lasound

resp_err : src/resp_err.c
	$(CC) src/resp_err.c -o resp_err

data_static : Makefile src/data_static.c static/style_volume.css static/script_volume.js static/favicon180.png
	xxd -i static/style_volume.css style_volume_css.c
	xxd -i static/script_volume.js script_volume_js.c
	xxd -i static/favicon180.png favicon180.c
	$(CC) src/data_static.c style_volume_css.c script_volume_js.c favicon180.c -o data_static $(STREAMER_DEFINES)
	rm *.c

system_play_bad : Makefile src/system_play_bad.c src/data_list.c src/data_list.h
	$(CC) src/system_play_bad.c src/data_list.c -o system_play_bad $(STREAMER_DEFINES) -lasound -lavcodec -lavutil -lswresample

system_play : Makefile src/system_play.c src/data_list.c src/data_list.h
	$(CC) src/system_play.c src/data_list.c -o system_play $(STREAMER_DEFINES) -lasound

system_volume : Makefile src/system_volume.c
	$(CC) src/system_volume.c -o system_volume $(STREAMER_DEFINES) -lasound

endif

.PHONY: err
err:
	$(error "STREAMER_PATH")

.PHONY: clean
clean:
	rm $(shell find . -maxdepth 1 -type f -not -name Makefile)

.PHONY: format
format:
	clang-format -i src/*
