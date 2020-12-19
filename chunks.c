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
	uint16_t pulsesPerQuarterNote = 24;
	uint8_t  framesPerSecond = 0;
	uint8_t  pulsesPerFrame = 0;

	format = getc(inputFilePointer) << 8;
	format |= getc(inputFilePointer);
	numberOfTracks = getc(inputFilePointer) << 8;
	numberOfTracks |= getc(inputFilePointer);
	division = getc(inputFilePointer) << 8;
	division |= getc(inputFilePointer);

	if (division & 0b1000000000000000) {

		/*
		 * Division = SMPTE
		 */

		divisionType = DIVISION_TYPE_SMPTE;
		framesPerSecond = (division >> 8) & 0b01111111; /* TODO */
		pulsesPerFrame = division & 0b11111111; /* TODO */
	} else {

		/*
		 * Division = PPQN
		 */

		divisionType = DIVISION_TYPE_PPQN; /* Yes, this is redundant, as I've already set it as the default anyway */
		pulsesPerQuarterNote = division;
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
		printf("\tDivision: %i frames per second, %i pulses per frame\n", framesPerSecond, pulsesPerFrame);
	} else {
		printf("\tDivision: %i pulses per quarter note\n", pulsesPerQuarterNote);
	}

	/*
	 * Discard remaining Header Chunk bytes, if there are more than 6
	 */

	uint32_t position = 6;

	for (position = 6; position < chunkLength; position++) {
		getc(inputFilePointer);
	}
}

/* See midi.pdf page 134, "Track Chunks" */

void readTrackChunk(FILE *inputFilePointer, uint32_t chunkLength) {
	uint32_t ticks = 0;

	for (positionInTrack = 0; positionInTrack < chunkLength; positionInTrack++) {
		ticks = readVariableLengthQuantity(inputFilePointer);
		printf("\t%i ticks in: <event>\n", ticks);
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