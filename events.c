/*
 * Event-level functions
 */

/* See midi.pdf page 35, "Data Format" */

void readEvent(FILE *inputFilePointer, uint32_t *position, uint8_t *status, uint8_t *pitchBendSensitivityInSemitones, uint8_t *pitchBendSensitivityInCents) {

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
		case 0x00: /* Standard sysex event */
		case 0x07: /* Non-standard sysex event */
			readSystemExclusiveMessage(inputFilePointer, position);
			return;

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
		}
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

	interpretMIDIEvent(status, statusNibbles, dataBytes, &pitchBendSensitivityInSemitones[0], &pitchBendSensitivityInCents[0]); /* Only interpret, as we've already read the bytes */
}