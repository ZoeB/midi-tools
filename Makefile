all:
	gcc midiread.c -o midiread
	./midiread fluoride.mid
