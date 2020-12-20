/*
 * MIDIread, a Standard MIDI File reader by Zoë Blade, 2020
 * (to teach myself the format).  Public domain.
 */

#include <stdint.h>
#include <stdio.h>

/*
 * MIDI handles one thing at a time, so it should be safe and most efficient
 * to use (and reuse) these global variables.  This also avoids pointers,
 * which have never been my forté.
 */

uint32_t position = 0;
uint8_t  status;

#include "tracks.c"
#include "chunks.c"

/*
 * File-level functions
 */

/* See midi.pdf page 132, "Chunks" */

void readFile(FILE *inputFilePointer) {
	int16_t  chunkType[4] = {'\0', '\0', '\0', '\0'}; /* Not uint8_t, as it might be EOF, which is -1 */
	uint32_t chunkLength = 0;

	while (1) {
		chunkType[0] = getc(inputFilePointer);

		if (chunkType[0] == EOF) {
			return;
		}

		chunkType[1] = getc(inputFilePointer);
		chunkType[2] = getc(inputFilePointer);
		chunkType[3] = getc(inputFilePointer);

		chunkLength = getc(inputFilePointer) << 24;
		chunkLength |= getc(inputFilePointer) << 16;
		chunkLength |= getc(inputFilePointer) << 8;
		chunkLength |= getc(inputFilePointer);

		if (chunkType[0] == 'M' && chunkType[1] == 'T' && chunkType[2] == 'h' && chunkType[3] == 'd') {
			printf("Header chunk, %i bytes\n", chunkLength);
			readHeaderChunk(inputFilePointer, chunkLength);
		} else if (chunkType[0] == 'M' && chunkType[1] == 'T' && chunkType[2] == 'r' && chunkType[3] == 'k') {
			printf("Track chunk, %i bytes\n", chunkLength);
			readTrackChunk(inputFilePointer, chunkLength);
		} else {
			printf("Unknown chunk, %i bytes\n", chunkLength);
			readUnknownChunk(inputFilePointer, chunkLength);
		}
	}
}

/*
 * OS-level functions
 */

int main(int argc, char *argv[]) {
	FILE *filePointer;

	if (argc == 1) {
		readFile(stdin);
	} else {
		while (--argc > 0) {
			filePointer = fopen(*++argv, "r");

			if (filePointer == NULL) {
				continue;
			}

			readFile(filePointer);
			fclose(filePointer);
		}
	}

	return 0;
}