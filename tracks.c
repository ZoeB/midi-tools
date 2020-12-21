/*
 * Track-level functions
 */

/* See midi.pdf page 131, "Conventions" */

uint32_t readVariableLengthQuantity(FILE *inputFilePointer, uint32_t *position) {

	/*
	 * Read a 1 to 4 byte number
	 */

	uint32_t quantity = 0;
	uint8_t  byte = 0;

	do {
		byte = getc(inputFilePointer);
		(*position)++;
		quantity <<= 7;
		quantity |= byte;
	} while (byte & 0b10000000);

	return quantity;
}

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

void readSystemExclusiveMessage(FILE *inputFilePointer, uint32_t *position) {

	/*
	 * System Exclusive Message start, with variable data length
	 * This starts with the length.  Theoretically, we can ignore that and simply look look for the F7 status that signifies the end of the System Exclusive Message, but that plan's scuppered: some cheeky MIDI devices use F7 to mean "begin additional System Exclusive Message" instead of "end previous System Exclusive Message".  So we need to read the number of bytes to skip, and do so!
	 * See midi.pdf page 135, "<sysex event>..."
	 */

	uint32_t bytesToSkip = 0;

/* Don't do this!  See comment above.
	while (getc(inputFilePointer) != 0xF7) {
		(*position)++;
	}

	(*position)++;
	return;
*/

	bytesToSkip = readVariableLengthQuantity(inputFilePointer, position);

	while (bytesToSkip > 0) {
		getc(inputFilePointer);
		(*position)++;
		bytesToSkip--;
	}
}

/* See midi.pdf page 35, "Data Format" */

void readEvent(FILE *inputFilePointer, uint32_t *position, uint8_t *status) {

	/*
	 * Most MIDI events consist of 1 to 3 bytes: a status byte followed by 0 to 2 data bytes.
	 * There are exceptions:
	 * System Exclusive Messages can have more data bytes.
	 * A running status allows us to skip a status byte entirely, and dive straight into data bytes!
	 * See midi.pdf page 100, "Table I: Summary of Status Bytes"
	 */

	uint8_t  byte = 0;
	/* uint8_t status is initialised in readTrackChunk() as it can persist through multiple calls to readEvent(), as a "running status" */
	uint8_t  statusNibbles[2] = {0, 0};
	uint8_t  dataBytes[2] = {0, 0};
	uint8_t  dataBytesRequired = 0;
	uint8_t  dataBytesRead = 0;
	uint32_t bytesToSkip = 0; /* For System Exclusive Messages */

	char noteLetter[12] = "CCDDEFFGGAAB";
	char noteIntonation[12] = "-#-#--#-#-#-";
	int octave;
	int pitch;

	byte = getc(inputFilePointer);
	(*position)++;

	if (byte & 0b10000000) {

		/*
		 * The first byte is a status.  Update the running status.
		 */

		*status = byte;
		dataBytesRead = 0;
	} else {

		/*
		 * The first byte is data.  Save it.  Keep the running status from the last event.
		 */

		dataBytes[0] = byte;
		dataBytesRead = 1;
	}

	/*
	 * Split up the status into two nibbles, as Channel Voice Messages use the first nibble for the command, and the second for the channel
	 * See midi.pdf page 100, "Table I: Summary of Status Bytes"
	 */

	statusNibbles[0] = *status >> 4;
	statusNibbles[1] = *status & 0x0F;

	switch (statusNibbles[0]) {
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0E:

		/*
		 * Channel Voice Messages / Channel Mode Messages, with set data lengths
		 */

		dataBytesRequired = 2;
		break;

	case 0x0C:
	case 0x0D:

		/*
		 * More Channel Voice Messages, with set data lengths
		 */

		dataBytesRequired = 1;
		break;

	case 0x0F:

		/*
		 * System Messages are edge cases, with various data lengths
		 * See midi.pdf page 105, "Table V: System Common Messages"
		 */

		switch (statusNibbles[1]) {
		case 0x00:

			/*
			 * System Exclusive Message start, with variable data length
			 * This starts with the length.  Theoretically, we can ignore that and simply look look for the F7 status that signifies the end of the System Exclusive Message, but that plan's scuppered: some cheeky MIDI devices use F7 to mean "begin additional System Exclusive Message" instead of "end previous System Exclusive Message".  So we need to read the number of bytes to skip, and do so!
			 * See midi.pdf page 135, "<sysex event>..."
			 */

			readSystemExclusiveMessage(inputFilePointer, position);
			return;
			break; /* Clearly, this is also redundant, but generally good practice */

		case 0x01:
		case 0x03:

			/*
			 * System Common Messages, with set data lengths
			 */

			dataBytesRequired = 1;
			break;

		case 0x02:

			/*
			 * More System Common Messages, with set data lengths
			 */

			dataBytesRequired = 2;
			break;

		case 0x04: /* Undefined System Common Message */
		case 0x05: /* Undefined System Common Message */
		case 0x06: /* System Common Message with no data bytes */
		case 0x07: /* System Exclusive Message end, with no data bytes (this shouldn't happen, as this byte should only be received within the SysEx loop above) */

			dataBytesRequired = 0; /* This is redundant, just to clarify */
			break;

		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:

			/*
			 * System Real Time Messages, with no data
			 * See midi.pdf page 106, "Table VI: System Real Time Messages"
			 */

			dataBytesRequired = 0; /* This is redundant, just to clarify */
			break;

		case 0x0F:

			/*
			 * In a live MIDI stream, a status of FF is the System Real Time Message of System Reset.
			 * In a stored MIDI file, that's not necessary, so it's repurposed for Meta-Events.
			 * See midi.pdf page 137, "Meta-Events" (which doesn't mention that, and perhaps should)
			 */

			readMetaEvent(inputFilePointer, position);
			return;
			break; /* Clearly, this is also redundant, but generally good practice */
		}

		break;

	default:
		/* This shouldn't happen! */
		break;
	}

	/*
	 * We know how many bytes we've read, and how many are required,
	 * so get the remaining ones
	 */

	while (dataBytesRequired > dataBytesRead) {
		dataBytes[dataBytesRead] = getc(inputFilePointer);
		(*position)++;
		dataBytesRead++;
	}

	switch (statusNibbles[0]) {
	case 0x08:
	case 0x09:

		if (statusNibbles[0] == 0x08) {
			printf("Note-Off, ");
		} else { /* statusNibbles[0] == 0x09 */
			printf("Note-On, ");
		}

		octave = (dataBytes[0] / 12) - 1;
		pitch = dataBytes[0] % 12;

		printf("channel %Xh, %c%c%i, velocity %02Xh\n", statusNibbles[1], noteLetter[pitch], noteIntonation[pitch], octave, dataBytes[1]);
		break;

	case 0x0B:
		printf("Control Change, channel %Xh, controller %02Xh, value %02Xh\n", statusNibbles[1], dataBytes[0], dataBytes[1]);
		break;
	default:
		printf("status %02Xh, data %02Xh %02Xh\n", *status, dataBytes[0], dataBytes[1]);
	}
}