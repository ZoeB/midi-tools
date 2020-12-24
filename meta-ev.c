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
	bytesLeft = readVariableLengthQuantity(inputFilePointer, position);

	if (metaEventType > 0x00 && metaEventType < 0x0A) {

		/*
		 * Plain text meta-event
		 */

		switch (metaEventType) {
		case 0x01:
			printf("Text Event: ");
			break;

		case 0x02:
			printf("Copyright Notice: ");
			break;

		case 0x03:
			printf("Sequence/Track Name: ");
			break;

		case 0x04:
			printf("Instrument Name: ");
			break;

		case 0x05:
			printf("Lyric: ");
			break;

		case 0x06:
			printf("Marker: ");
			break;

		case 0x07:
			printf("Cue Point: ");
			break;

		case 0x09: /* TODO: verify this with official documentation */
			printf("Device Name: ");
			break;

		default:
			printf("Unknown plain text: ");
		}

		/*
		 * Simply display the text verbatim
		 */

		while (bytesLeft > 0) {
			printf("%c", getc(inputFilePointer));
			(*position)++;
			bytesLeft--;
		}

		printf("\n");
		return;
	}

	switch (metaEventType) {
	case 0x21: /* Port  TODO: verify this with official documentation */
		if (bytesLeft != 1) {
			printf("(error: length 1 expected, %i received) ", bytesLeft);
		}

		byte = getc(inputFilePointer);
		(*position)++;
		printf("Port %02Xh\n", byte);
		return;

	case 0x2F: /* End of Track */
		if (bytesLeft != 0) {
			printf("(error: length 0 expected, %i received) ", bytesLeft);
		}

		printf("End of Track\n"); /* TODO: display the actual signature */
		return;

	case 0x51: /* Set Tempo (See midi.pdf page 138, "Set Tempo") */
		if (bytesLeft != 3) {
			printf("(error: length 3 expected, %i received) ", bytesLeft);
		}

		tempo |= getc(inputFilePointer);
		(*position)++;
		tempo <<= 8;
		tempo |= getc(inputFilePointer);
		(*position)++;
		tempo <<= 8;
		tempo |= getc(inputFilePointer);
		(*position)++;
		printf("Tempo %i microseconds per quarter-note (%i BPM)\n", tempo, 60000000 / tempo);
		return;

	case 0x58: /* Time Signature (See midi.pdf page 139, "Time Signature") */
		if (bytesLeft != 4) {
			printf("(error: length 4 expected, %i received) ", bytesLeft);
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
		if (bytesLeft != 3) {
			printf("(error: length 3 expected, %i received) ", bytesLeft);
		}

		break;

	case 0x20: /* MIDI Channel Prefix */
		if (bytesLeft != 2) {
			printf("(error: length 2 expected, %i received) ", bytesLeft);
		}

		break;

	case 0x01: /* Text Event */
	case 0x02: /* Copyright Notice */
	case 0x05: /* Lyric */
	case 0x07: /* Cue Point */
	case 0x7F: /* Sequencer-Specific Meta-Event */
		break;

	case 0x54: /* SMPTE Offset */
		if (bytesLeft != 6) {
			printf("(error: length 6 expected, %i received) ", bytesLeft);
		}

		break;

	case 0x59: /* Key Signature */
		if (bytesLeft != 3) {
			printf("(error: length 3 expected, %i received) ", bytesLeft);
		}

		break;
	}

	printf("type %02Xh, %04i bytes long\n", metaEventType, bytesLeft);

	while (bytesLeft > 0) {
		getc(inputFilePointer);
		(*position)++;
		bytesLeft--;
	}
}