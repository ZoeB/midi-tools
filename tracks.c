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

	uint8_t  byte = 0;
	uint8_t  lastByteRead = 0; /* 0 = we don't know anything; 1 = we know the status; 2 = we know dataA; 3 = we know dataB */
	/* status is initialised in readTrackChunk() as it can persist through multiple calls to readEvent(), as a "running status" */
	uint8_t  dataA = 0;
	uint8_t  dataB = 0;

	byte = getc(inputFilePointer);
	position++;

	if (byte & 0b10000000) {

		/*
		 * Change the status
		 */

		status = byte;
		lastByteRead = 1;
	} else {

		/*
		 * Keep the running status from the last event
		 */

		dataA = byte;
		lastByteRead = 2;
	}

	/* TODO: Switch or if-else-if-etc based on the status to determine the data length etc */

	printf("status %02X, data %02X\n", status, dataA);
}