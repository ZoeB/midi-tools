/*
 * Meta-event-level functions
 */

void readMetaEvent(FILE *inputFilePointer, uint32_t *position) {

	/*
	 * In a live MIDI stream, a status of FF is the System Real Time Message of 
	 * System Reset.  In a stored MIDI file, that's not necessary, so
	 * it's repurposed for Meta-Events.  See midi.pdf page 137, "Meta-Events"
	 * (which doesn't mention repurposing the System Reset, and perhaps should)
	 */

	/* TODO: display these values instead of skipping them */
	/* TODO: the "expected" errors are lengths!  So rewrite this bearing that in mind.  I thought it would be simpler if *all* Meta-Events had specified variable lengths, and it looks like that's already the case.  "...the lengths of events which do not have a variable amount of data are given directly in hexadecimal."  It says it's only a single byte length, but that's because it's less than 128 anyway, so another way to look at it is that it is a variable length that happens not to vary.  So let's universally call readVariableLengthQuantity() here! */

	uint8_t  metaEventType = 0;
	uint8_t  byte = 0;
	uint32_t bytesLeft = 0;

	/* For 0x51: Set Tempo */
	uint32_t tempo = 0;

	/* For 0x58: Time Signature */
	uint8_t  numerator = 0;
	uint8_t  denominator = 0;
	uint8_t  ticksPerMetronomeClick = 0;
	uint8_t  thirtySecondNotesPerQuarterNote = 0;

	printf("meta-event: ");

	metaEventType = getc(inputFilePointer);
	(*position)++;

	switch (metaEventType) {
	case 0x03: /* Sequence/Track Name */
		printf("Sequence/Track Name: ");
		bytesLeft = readVariableLengthQuantity(inputFilePointer, position);

		while (bytesLeft > 0) {
			printf("%c", getc(inputFilePointer));
			(*position)++;
			bytesLeft--;
		}

		printf("\n");
		return;

	case 0x04: /* Instrument Name */
		printf("Instrument Name: ");
		bytesLeft = readVariableLengthQuantity(inputFilePointer, position);

		while (bytesLeft > 0) {
			printf("%c", getc(inputFilePointer));
			(*position)++;
			bytesLeft--;
		}

		printf("\n");
		return;

	case 0x21: /* Port  TODO: verify this with official documentation.  People on forums and online guides report that it's the MIDI Port, but I'd like to see it in an official spec if possible. */
		byte = getc(inputFilePointer);
		(*position)++;

		if (byte != 01) {
			printf("(error: 01h expected, %02Xh received) ", byte);
		}

		byte = getc(inputFilePointer);
		(*position)++;
		printf("Port %02Xh\n", byte);
		return;

	case 0x2F: /* End of Track */
		byte = getc(inputFilePointer);
		(*position)++;

		if (byte != 00) {
			printf("(error: 00h expected, %02Xh received) ", byte);
		}

		printf("End of Track\n"); /* TODO: display the actual signature */
		return;

	case 0x51: /* Set Tempo (See midi.pdf page 138, "Set Tempo") */

		byte = getc(inputFilePointer);
		(*position)++;

		if (byte != 03) {
			printf("(error: 03h expected, %02Xh received) ", byte);
		}

		tempo |= getc(inputFilePointer);
		(*position)++;
		tempo <<= 8;
		tempo |= getc(inputFilePointer);
		(*position)++;
		tempo <<= 8;
		tempo |= getc(inputFilePointer);
		(*position)++;
		printf("Tempo %06Xh microseconds per quarter-note\n", tempo);
		return;

	case 0x58: /* Time Signature (See midi.pdf page 139, "Time Signature") */
		byte = getc(inputFilePointer);
		(*position)++;

		if (byte != 04) {
			printf("(error: 04h expected, %02Xh received) ", byte);
		}

		numerator = getc(inputFilePointer);
		(*position)++;

		denominator = 1 << getc(inputFilePointer);
		(*position)++;

		ticksPerMetronomeClick = getc(inputFilePointer);
		(*position)++;

		thirtySecondNotesPerQuarterNote = getc(inputFilePointer);
		(*position)++;

		printf("Time Signature %i/%i\n", numerator, denominator); /* TODO: try to work out a simple way to describe the other two values */
		return;

/* TODO: implement these properly rather than simply skipping them */

	case 0x00: /* Sequence Number */
		bytesLeft = 3;
		break;

	case 0x20: /* MIDI Channel Prefix */
		bytesLeft = 2;
		break;

	case 0x01: /* Text Event */
	case 0x02: /* Copyright Notice */
	case 0x05: /* Lyric */
	case 0x06: /* Marker */
	case 0x07: /* Cue Point */
	case 0x7F: /* Sequencer-Specific Meta-Event */
		bytesLeft = readVariableLengthQuantity(inputFilePointer, position);
		break;

	case 0x54: /* SMPTE Offset */
		bytesLeft = 6;
		break;

	case 0x59: /* Key Signature */
		bytesLeft = 3;
		break;
	}

	printf("type %02Xh, %04i bytes long\n", metaEventType, bytesLeft);

	while (bytesLeft > 0) {
		getc(inputFilePointer);
		(*position)++;
		bytesLeft--;
	}
}