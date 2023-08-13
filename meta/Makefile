MET_PATH := /home/exe/meta
METDEFS := -D exec_picture_getter_path=\"$(MET_PATH)/getpicture\"
CC := gcc -O0

all : meta getpicture
	mv meta getpicture $(MET_PATH)

meta : meta.c
	$(CC) $(METDEFS) meta.c -o meta

getpicture : getpicture.c
	$(CC) getpicture.c -o getpicture -lFLAC

.PHONY : clean
clean :
	rm -f meta getpicture
