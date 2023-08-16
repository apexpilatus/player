MET_PATH := /home/drive/meta
METDEFS := -D picture_getter_path=\"$(MET_PATH)/getpicture\" -D tags_getter_path=\"$(MET_PATH)/gettags\"
CC := gcc -O0

all : meta getpicture gettags
	mv meta getpicture gettags $(MET_PATH)

meta : meta.c shares.c shares.h
	$(CC) $(METDEFS) meta.c shares.c -o meta

getpicture : getpicture.c shares.c shares.h
	$(CC) getpicture.c shares.c -o getpicture -lFLAC

gettags : gettags.c shares.c shares.h
	$(CC) gettags.c shares.c -o gettags -lFLAC

.PHONY : clean
clean :
	rm -f meta getpicture gettags
