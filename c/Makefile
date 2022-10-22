player : waiter main.c
	gcc main.c -o player -Wl,-rpath=/home/player -L . -lplayer -lasound -lFLAC
	mv libplayer.so waiter player /home/player

waiter : libplayer.so waiter.c
	gcc waiter.c -o waiter -Wl,-rpath=/home/player -L . -lplayer -lasound -lFLAC

libplayer.so : funcs.c funcs.h
	gcc -shared funcs.c -o libplayer.so

.PHONY : clean
clean :
	rm -rf libplayer.so player waiter
