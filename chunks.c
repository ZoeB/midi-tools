/*
 * Chunk-level functions
 */

/* See midi.pdf page 133, "Header Chunks" */

void readHeaderChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	if (chunkLength < 6) {
		return; /* This shouldn't happen */
	}

	#define FORMAT_SINGLE_TRACK 0
	#define FORMAT_MULTIPLE_SIMULTANEOUS_TRACKS 1
	#define FORMAT_MULTIPLE_INDEPENDENT_TRACK 2

	#define DIVISION_TYPE_PPQN 0
	#define DIVISION_TYPE_SMPTE 1

	uint16_t format = 0;
	uint16_t numberOfTracks = 0;
	uint16_t division = 0;

	uint8_t  divisionType = DIVISION_TYPE_PPQN;
	uint16_t ticksPerQuarterNote = 24;
	uint8_t  framesPerSecond = 0;
	uint8_t  ticksPerFrame = 0;

	format = getc(inputFilePointer) << 8;
	format |= getc(inputFilePointer);
	numberOfTracks = getc(inputFilePointer) << 8;
	numberOfTracks |= getc(inputFilePointer);
	division = getc(inputFilePointer) << 8;
	division |= getc(inputFilePointer);
	position += 6;

	if (division & 0b1000000000000000) {

		/*
		 * Division = SMPTE (hours, minutes, seconds, frames, ticks per frame)
		 */

		divisionType = DIVISION_TYPE_SMPTE;
		framesPerSecond = (division >> 8) & 0b01111111; /* TODO */
		ticksPerFrame = division & 0b11111111; /* TODO */
	} else {

		/*
		 * Division = PPQN (pulses/ticks per quarter note)
		 */

		divisionType = DIVISION_TYPE_PPQN; /* Yes, this is redundant, as I've already set it as the default anyway */
		ticksPerQuarterNote = division;
	}

	switch (format) {
	case FORMAT_SINGLE_TRACK:
		printf("\tFormat: single track\n");
		break;

	case FORMAT_MULTIPLE_SIMULTANEOUS_TRACKS:
		printf("\tFormat: multiple simultaneous tracks\n");
		break;

	case FORMAT_MULTIPLE_INDEPENDENT_TRACK:
		printf("\tFormat: multiple independent tracks\n");
	}

	printf("\tNumber of tracks: %i\n", numberOfTracks);

	if (divisionType == DIVISION_TYPE_SMPTE) {
		printf("\tDivision: %i frames per second, %i ticks per frame\n", framesPerSecond, ticksPerFrame);
	} else {
		printf("\tDivision: %i ticks per quarter note\n", ticksPerQuarterNote);
	}

	/*
	 * Discard remaining Header Chunk bytes, if there are more than 6
	 */

	while (position < chunkLength) {
		getc(inputFilePointer);
		position++;
	}
}

/* See midi.pdf page 134, "Track Chunks" */

void readTrackChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t ticks = 0;
	uint8_t  *status = 0; /* Initialised here, as it can persist from one
	                       * MIDI event to the next, a "running status" */

	while (position < chunkLength) {
		ticks = readVariableLengthQuantity(inputFilePointer);
		printf("\t%04i more ticks in: ", ticks);
		readEvent(inputFilePointer, status);
	}
}

/* "Skip" might be more accurate than "read". */

void readUnknownChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t position = 0;

	for (position = 0; position < chunkLength; position++) {
		getc(inputFilePointer);
	}
}