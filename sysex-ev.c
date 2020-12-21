/*
 * Sysex event-level functions
 */

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