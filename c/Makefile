PL_PATH := /home/exe/player

CC := gcc -O2 -Wno-unused-result

PLAYDEFS := -D exec_player_path=\"$(PL_PATH)/player\" -D exec_mixer_path=\"$(PL_PATH)/mixer\"
PLAYLIBS := -lFLAC

PLAYSRC := wilkins.c
PLAYDEFS += -D card_name=\"Wilkins\"
PLAYLIBS += -lswresample -lavutil -lavcodec

all : waiter mixer player
	mv waiter player mixer $(PL_PATH)

% : %.c
	$(CC) $< -o $@ -lasound $(if $(findstring waiter, $<), $(PLAYDEFS))

player : $(PLAYSRC)
	$(CC) $(PLAYSRC) -o player -lasound $(PLAYLIBS)

.PHONY : clean
clean :
	rm -rf player waiter mixer
