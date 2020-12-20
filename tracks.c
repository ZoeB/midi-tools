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
	 * (which doesn't mention that, and perhaps should)
	 */

	/* TODO: display these values instead of skipping them */

	uint8_t  metaEventType = 0;
	uint32_t bytesToSkip = 0;

	metaEventType = getc(inputFilePointer);
	(*position)++;

	switch (metaEventType) {
	case 0x00: /* Sequence Number */
	case 0x21: /* TODO: find out what this is.  Reason seems to output it.  People on forums and online guides report that it's the MIDI Port, but I'd like to see it in an official spec if possible.  It seems to take 3 bytes in Reason's output, although allegedly it only needs 2. */
		bytesToSkip = 3;
		break;

	case 0x20: /* MIDI Channel Prefix */
		bytesToSkip = 2;
		break;

	case 0x51: /* Set Tempo */
		bytesToSkip = 4;
		break;

	case 0x01: /* Text Event */
	case 0x02: /* Copyright Notice */
	case 0x03: /* Sequence/Track Name */
	case 0x04: /* Instrument Name */
	case 0x05: /* Lyric */
	case 0x06: /* Marker */
	case 0x07: /* Cue Point */
	case 0x7F: /* Sequencer-Specific Meta-Event */
			bytesToSkip = readVariableLengthQuantity(inputFilePointer, &position);
			break;

	case 0x2F: /* End of Track */
		bytesToSkip = 1;
		break;

	case 0x54: /* SMPTE Offset */
		bytesToSkip = 6;
		break;

	case 0x58: /* Time Signature */
		bytesToSkip = 5;
		break;

	case 0x59: /* Key Signature */
		bytesToSkip = 3;
		break;
	}

	printf("meta event type %02X, %04i bytes long\n", metaEventType, bytesToSkip);

	while (bytesToSkip > 0) {
		getc(inputFilePointer);
		(*position)++;
		bytesToSkip--;
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

	bytesToSkip = readVariableLengthQuantity(inputFilePointer, &position);

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

			readSystemExclusiveMessage(inputFilePointer, &position);
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

			readMetaEvent(inputFilePointer, &position);
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

	printf("status %02X, data %02X %02X\n", *status, dataBytes[0], dataBytes[1]);
}