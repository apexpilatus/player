PL_PATH := /home/exe/player
CC := clang -O2

all : waiter mixer player
	mv waiter player mixer $(PL_PATH)

player : player.c
	$(CC) player.c -o player -lasound -lFLAC -lswresample -lavutil -lavcodec

waiter : waiter.c
	$(CC) waiter.c -o waiter -lasound

mixer : mixer.c
	$(CC) mixer.c -o mixer -lasound

.PHONY : clean
clean :
	rm -rf player waiter mixer
