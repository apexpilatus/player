PL_PATH := /home/exe/player
CC := clang -O2

player : waiter mixer libplayer.so player.c
	$(CC) player.c -o player -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC -lswresample -lavutil -lavcodec
	mv libplayer.so waiter player mixer $(PL_PATH)

waiter : libplayer.so waiter.c
	$(CC) waiter.c -o waiter -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC

libplayer.so : funcs.c funcs.h
	$(CC) -shared funcs.c -o libplayer.so

mixer : mixer.c
	$(CC) mixer.c -o mixer -lasound

.PHONY : clean
clean :
	rm -rf libplayer.so player waiter
