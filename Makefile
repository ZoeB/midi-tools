all:
	gcc midiread.c -o midiread
	./midiread test.mid | less
