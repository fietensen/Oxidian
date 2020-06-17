# Build on Linux

IDIR=./src
ODIR=/usr/bin
LDIR=$(IDIR)/lib
CC=gcc

BASE=$(IDIR)/main.c $(IDIR)/VM.c $(IDIR)/environment.c $(IDIR)/functions.c $(IDIR)/helper_functions.c $(IDIR)/errors.c
LIBRARIES=$(LDIR)/lib_empty.c $(LDIR)/lib_io.c $(LDIR)/lib_mem.c $(LDIR)/lib_net.c

FILES=$(BASE) $(LIBRARIES)

CFLAGS=-Wall -O2 -std=c99 -I./include
DBGFLAGS=-g3 -g -fsanitize=address
# remove comments to add additional functionality:
#CFLAGS+=-lssl

Oxidian:
	$(CC) $(CFLAGS) -o $(ODIR)/Oxidian $(FILES)
	cp ./olc.py $(ODIR)/olc.py
	chmod 755 $(ODIR)/olc.py
	mkdir /usr/local/OxyLibs
	cp OxyLibs/* /usr/local/OxyLibs

debug:
	$(CC) $(CFLAGS) $(DBGFLAGS) -o $(ODIR)/Oxidian $(FILES)
	cp $(IDIR)/olc.py $(ODIR)/olc.py
	chmod 755 $(ODIR)/olc.py
	mkdir /usr/local/OxyLibs
	cp OxyLibs/* /usr/local/OxyLibs

OxyLibs: /usr/local/OxyLibs
	rm -rf /usr/local/OxyLibs/*
	cp libs/* /usr/local/OxyLibs

remove: /usr/local/OxyLibs $(ODIR)/Oxidian $(ODIR)/olc.py
	rm -rf /usr/local/OxyLibs $(ODIR)/Oxidian $(ODIR)/olc.py
