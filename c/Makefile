PL_PATH := /home/exe/player

CC := gcc -O2

PLAYDEFS := -D exec_player_path=\"$(PL_PATH)/player\" -D exec_mixer_path=\"$(PL_PATH)/mixer\"
PLAYLIBS := -lFLAC -lasound

ifeq ($(target), shure)
PLAYSRC := shure.c
PLAYDEFS += -D card_name=\"S50\"
else
$(error "target" should be shure)
endif

all : waiter mixer player meta
	mv waiter player mixer meta $(PL_PATH)

waiter : waiter.c shares.h
	$(CC) $(PLAYDEFS) waiter.c -o waiter -lasound

mixer : mixer.c shares.h
	$(CC) mixer.c -o mixer -lasound

player : $(PLAYSRC) playlib.c playlib.h
	$(CC) $(PLAYSRC) playlib.c -o player $(PLAYLIBS)

meta : meta.c
	$(CC) meta.c -o meta -lFLAC

.PHONY : clean
clean :
	rm -f player waiter mixer
