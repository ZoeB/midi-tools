/*
 * MIDIread, a Standard MIDI File reader by Zoë Blade, 2020
 * (to teach myself the format).  Public domain.
 */

#include <stdint.h>
#include <stdio.h>

/* See midi.pdf page 133 */

void readHeaderChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t position = 0;

	for (position = 0; position < chunkLength; position++) {
		getc(inputFilePointer);
	}
}

/* See midi.pdf page 132 */

void readTrackChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t position = 0;

	for (position = 0; position < chunkLength; position++) {
		getc(inputFilePointer);
	}
}

/* "Skip" might be more accurate than "read". */

void readUnknownChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t position = 0;

	for (position = 0; position < chunkLength; position++) {
		getc(inputFilePointer);
	}
}

/* See midi.pdf page 132 */

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