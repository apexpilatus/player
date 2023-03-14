PL_PATH := /home/exe/player

CC := gcc -O2

PLAYDEFS := -D exec_player_path=\"$(PL_PATH)/player\" -D exec_mixer_path=\"$(PL_PATH)/mixer\"
PLAYLIBS := -lFLAC -lasound

PLAYSRC := wilkins.c
PLAYDEFS += -D card_name=\"Wilkins\"
PLAYLIBS += -lswresample -lavutil -lavcodec

all : waiter mixer player
	mv waiter player mixer $(PL_PATH)

waiter : waiter.c shares.h
	$(CC) $(PLAYDEFS) waiter.c -o waiter -lasound

mixer : mixer.c shares.h
	$(CC) mixer.c -o mixer -lasound

player : $(PLAYSRC) playlib.c playlib.h
	$(CC) $(PLAYSRC) playlib.c -o player $(PLAYLIBS)

.PHONY : clean
clean :
	rm -rf player waiter mixer
