/*
 * Global functions, used at various levels
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