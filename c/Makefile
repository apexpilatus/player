PL_PATH := /home/exe/player

CC := gcc -O2

PLAYSRC := wilkins.c
PLAYLIBS := -lFLAC -lswresample -lavutil -lavcodec

all : waiter mixer player
	mv waiter player mixer $(PL_PATH)

% : %.c
	$(CC) $< -o $@ -lasound -lFLAC -lswresample -lavutil -lavcodec

.PHONY : clean
clean :
	rm -rf player waiter mixer
