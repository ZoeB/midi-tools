/*
 * Track-level functions
 */

/* See midi.pdf page 131, "Conventions" */

uint32_t readVariableLengthQuantity(FILE *inputFilePointer, *position) {

	/*
	 * Read a 1 to 4 byte number
	 */

	uint32_t quantity = 0;
	uint8_t  byte = 0;

	do {
		byte = getc(inputFilePointer);
		position++;
		quantity <<= 7;
		quantity |= byte;
	} while (byte & 0b10000000);

	return quantity;
}

/* See midi.pdf page 35, "Data Format" */

void readEvent(FILE *inputFilePointer, *position, *status) {

	/*
	 * Most MIDI events consist of 1 to 3 bytes: a status byte followed by 0 to 2 data bytes.
	 * There are exceptions:
	 * system exclusive messages (SysEx) can have more data bytes.
	 * A running status allows us to skip a status byte entirely, and dive straight into data bytes!
	 * See midi.pdf page 100, "Table I: Summary of Status Bytes"
	 */

	#define NOTHING_KNOWN = 0;
	#define STATUS_KNOWN = 1;
	#define DATA_A_KNOWN = 2;
	#define DATA_B_KNOWN = 3;

	uint8_t  byte = 0;
	uint8_t  lastByteRead = NOTHING_KNOWN;
	/* status is initialised in readTrackChunk() as it can persist through multiple calls to readEvent(), as a "running status" */
	uint8_t  statusNibbleA;
	uint8_t  statusNibbleB;
	uint8_t  dataByteA = 0;
	uint8_t  dataByteB = 0;
	uint8_t  eventByteLength = 0;

	byte = getc(inputFilePointer);
	position++;

	if (byte & 0b10000000) {

		/*
		 * Change the status
		 */

		status = byte;
		lastByteRead = STATUS_KNOWN;
	} else {

		/*
		 * Keep the running status from the last event
		 */

		dataByteA = byte;
		lastByteRead = DATA_A_KNOWN;
	}

	/*
	 * Split up the status into two nibbles, as Channel Voice Messages use the first nibble for the command, and the second for the channel
	 * See midi.pdf page 100, "Table I: Summary of Status Bytes"
	 */

	statusNibbleA = status >> 4;
	statusNibbleB = status & 0x0F;

	switch (statusNibbleA) {
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
	case 0x0E:
		eventByteLength = 3;
		break;

	case 0x0C:
	case 0x0D:
		eventByteLength = 2;
		break;

	case 0x0F:
		/* TODO: determine data length for these edge cases! */
		break;

	default:
		/* This shouldn't happen! */
	}

	/*
	 * We know how many bytes we've read, and how many are required,
	 * so get the remaining ones
	 */

	if ()

	printf("status %02X, data %02X\n", status, dataByteA);
}