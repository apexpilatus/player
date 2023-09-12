PL_PATH := /home/waiter

CC := clang -O2 -Wno-unused-result

PLAYDEFS := -D exec_player_path=\"$(PL_PATH)/player\" -D exec_mixer_path=\"$(PL_PATH)/mixer\"
PLAYLIBS := -lFLAC -lasound

all : waiter mixer player
	mv waiter player mixer /mnt$(PL_PATH)

waiter : waiter.c shares.h
	$(CC) $(PLAYDEFS) waiter.c -o waiter -lasound

mixer : mixer.c shares.h
	$(CC) mixer.c -o mixer -lasound

player : play.c
	$(CC) play.c -o player $(PLAYLIBS)

.PHONY : clean
clean :
	rm -f player waiter mixer
