PL_PATH := /home/exe/player
CC := gcc -O2

player : waiter main.c
	$(CC) main.c -o player -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC -lswresample -lavutil -lavcodec
	mv libplayer.so waiter player $(PL_PATH)

waiter : libplayer.so waiter.c
	$(CC) waiter.c -o waiter -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC

libplayer.so : funcs.c funcs.h
	$(CC) -shared funcs.c -o libplayer.so

.PHONY : clean
clean :
	rm -rf libplayer.so player waiter
