MET_PATH := /home/exe/meta
METDEFS := -D picture_getter_path=\"$(MET_PATH)/getpicture\" -D tags_getter_path=\"$(MET_PATH)/gettags\"
CC := gcc -O0

all : meta getpicture gettags
	mv meta getpicture gettags $(MET_PATH)

meta : meta.c
	$(CC) $(METDEFS) meta.c -o meta

getpicture : getpicture.c
	$(CC) getpicture.c -o getpicture -lFLAC

gettags : gettags.c
	$(CC) gettags.c -o gettags -lFLAC

.PHONY : clean
clean :
	rm -f meta getpicture gettags
