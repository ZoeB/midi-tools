/*
 * MIDIread, a Standard MIDI File reader by Zoë Blade, 2020
 * (to teach myself the format).  Public domain.
 */

#include <stdio.h>

/* This is mostly ripping off K&R's ANSI C, page 162, "Cat",
 * by way of my own Impulse Tracker and Akai filesystem
 * listing programs. */

void describeFile(FILE *inputFilePointer, FILE *outputFilePointer) {
	int character;

	while ((character = getc(inputFilePointer)) != EOF) {
		putc(character, outputFilePointer);
	}

	return;
}

int main(int argc, char *argv[]) {
	FILE *filePointer;

	if (argc == 1) {
		describeFile(stdin, stdout);
	} else {
		while (--argc > 0) {
			filePointer = fopen(*++argv, "r");

			if (filePointer == NULL) {
				continue;
			}

			describeFile(filePointer, stdout);
			fclose(filePointer);
		}
	}

	return 0;
}