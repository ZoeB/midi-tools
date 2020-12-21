/*
 * MIDI event-level functions
 */

void interpretMIDIEvent(uint8_t *status, uint8_t *statusNibbles, uint8_t *dataBytes) {
	char noteLetter[12] = "CCDDEFFGGAAB";
	char noteIntonation[12] = "-#-#--#-#-#-";
	int octave;
	int pitch;

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
		printf("Control Change, channel %Xh, controller %02Xh, value %02Xh\n", statusNibbles[1], dataBytes[0], dataBytes[1]); /* See midi.pdf page 102, "Table III: Controller Numbers" */
		break;
	default:
		printf("status %02Xh, data %02Xh %02Xh\n", *status, dataBytes[0], dataBytes[1]);
	}
}