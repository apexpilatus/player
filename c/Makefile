PL_PATH := /home/exe/player

player : waiter main.c
	gcc main.c -o player -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC
	mv libplayer.so waiter player $(PL_PATH)

waiter : libplayer.so waiter.c
	gcc waiter.c -o waiter -Wl,-rpath=$(PL_PATH) -L . -lplayer -lasound -lFLAC

libplayer.so : funcs.c funcs.h
	gcc -shared funcs.c -o libplayer.so

.PHONY : clean
clean :
	rm -rf libplayer.so player waiter
