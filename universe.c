/*
 * Universal functions
 */

/* See midi.pdf page 131, "Conventions" */

uint32_t readVariableLengthQuantity(FILE *inputFilePointer) {

	/*
	 * Read a 1 to 4 byte number
	 */

	uint32_t quantity = 0;
	uint8_t  byte = 0;

	do {
		byte = getc(inputFilePointer);
		positionInTrack++;
		quantity <<= 8;
		quantity |= byte;
	} while (byte & 0b10000000);

	return quantity;
}