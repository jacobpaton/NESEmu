CC=gcc-10

.PHONY: all run debug clean

all:
	make -C cpu all
	make -C ppu all

debug:
	make -C cpu debug
	make -C ppu debug

clean:
	make -C cpu clean
	make -C ppu clean
