all:
	gcc midiread.c -o midiread
	./midiread tests/test.mid | less
