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

/* TODO!  The most obvious solution here is to note, based on whether the first byte was a status or data, where we're up to, in terms of which byte it just was.  Then, based on the status, I can work out how many (more) data bytes to read.  But I should think about it for a day or two first, to see if any better solution becomes clear. */

	uint8_t  byte = 0;
	uint8_t  dataA = 0;
	uint8_t  dataB = 0;

	byte = getc(inputFilePointer);
	position++;

	if (byte & 0b10000000) {

		/*
		 * Change the status
		 */

		status = byte;

		if (status < 0xF8) {

			/*
			 * The status is NOT a system realtime message, so read at least one data byte
			 * See midi.pdf page 38, "Data Bytes"
			 * See midi.pdf page 62, "System Real Time Messages"
			 */

			dataA = getc(inputFilePointer);
		}

		position++;
	} else {

		/*
		 * Keep the running status from the last event
		 */

		dataA = byte;
	}

	printf("status %02X, data %02X\n", status, dataA);
}