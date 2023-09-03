MET_PATH := /home/meta
WT_PATH := /home/waiter
METDEFS := -D picture_getter_path=\"$(MET_PATH)/getpicture\" -D tags_getter_path=\"$(MET_PATH)/gettags\"
CC := clang -O2

ifdef init
METDEFS += -D exec_waiter_path=\"$(WT_PATH)/waiter\" -D META_INIT
ifdef debug
METDEFS += -D META_DEBUG
endif
endif

all : meta getpicture gettags
	mv meta getpicture gettags /mnt$(MET_PATH)
ifdef init
	sudo mv /mnt$(MET_PATH)/meta /mnt/sbin/init
endif

meta : meta.c shares.c shares.h
	$(CC) $(METDEFS) meta.c shares.c -o meta

getpicture : getpicture.c shares.c shares.h
	$(CC) getpicture.c shares.c -o getpicture -lFLAC

gettags : gettags.c shares.c shares.h
	$(CC) gettags.c shares.c -o gettags -lFLAC

.PHONY : clean
clean :
	rm -f meta getpicture gettags
