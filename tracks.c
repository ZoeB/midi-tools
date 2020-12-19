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