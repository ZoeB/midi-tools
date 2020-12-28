/*
 * MIDI event-level functions
 */

void interpretControllerNumber(uint8_t controllerNumber) {

	/*
	 * See midi.pdf page 102, "Table III: Controller Numbers"
	 */

	switch (controllerNumber) {
	case 0x00:
		printf("Bank Select");
		break;

	case 0x01:
		printf("Modulation wheel or lever");
		break;

	default:
		printf("controller %02Xh", controllerNumber);
	}
}

void interpretMIDIEvent(uint8_t *status, uint8_t *statusNibbles, uint8_t *dataBytes, uint8_t *pitchBendSensitivityInSemitones, uint8_t *pitchBendSensitivityInCents) {
	uint8_t  displayChannel = 1;
	char     noteLetter[12] = "CCDDEFFGGAAB";
	char     noteIntonation[12] = "-#-#--#-#-#-";
	uint8_t  octave = 0;
	uint8_t  pitch = 0;
	int16_t  pitchBend = 0;
	float    pitchBendRange = 2.0;
	float    pitchBendInSemitones = 0.0;

	displayChannel = statusNibbles[1] + 1;

	switch (statusNibbles[0]) {

	/*
	 * See midi.pdf page 41, "Channel Voice Messages"
	 * See midi.pdf page 100, "Table I: Summary of Status Bytes"
	 */

	case 0x08:
	case 0x09:
	case 0x0A:

		octave = (dataBytes[0] / 12) - 1;
		pitch = dataBytes[0] % 12;

		if (statusNibbles[0] == 0x08) {
			printf("Note-Off, channel %02i, %c%c%i, velocity %03i\n", displayChannel, noteLetter[pitch], noteIntonation[pitch], octave, dataBytes[1]);
		} else if (statusNibbles[0] == 0x09) {
			printf("Note-On, channel %02i, %c%c%i, velocity %03i\n", displayChannel, noteLetter[pitch], noteIntonation[pitch], octave, dataBytes[1]);
		} else { /* statusNibbles[0] == 0x0A */
			printf("Key Pressure, channel %02i, %c%c%i, value %03i\n", displayChannel, noteLetter[pitch], noteIntonation[pitch], octave, dataBytes[1]);
		}

		break;

	case 0x0B:
		printf("Control Change, channel %02i, ", displayChannel);
		interpretControllerNumber(dataBytes[0]);
		printf(", value %03i\n", dataBytes[1]);
		break;

	case 0x0C:
		printf("Program Change, channel %02i, value %03i\n", displayChannel, dataBytes[0]);
		break;

	case 0x0D:
		printf("Channel Pressure, channel %02i, value %03i\n", displayChannel, dataBytes[0]);
		break;

	case 0x0E: /* Pitch Bend (see page 51) */
		pitchBend = (dataBytes[1] << 7) | dataBytes[0]; /* Least significant byte, then most significant byte */
		pitchBend -= 0x2000; /* The spec defines the bytes 0x00 0x40 as neutral.  That's 0x40 0x00 with the most significant byte first; 0x20 0x00 when using all 8 bits, not just 7 of them.  So 0x2000 == neutral, neither sharp nor flat.  So subtract that to make the value bipolar, for a signed int. */
		pitchBendRange = *pitchBendSensitivityInSemitones + (*pitchBendSensitivityInCents / 100);
		pitchBendInSemitones = pitchBend / 0x1FFF * pitchBendRange; /* Instead of displaying +/- 8191, display the actual semitones and cents */
		printf("Pitch Bend, channel %02i, %+07.3f\n", displayChannel, pitchBendInSemitones);
		break;

	default:
		printf("status %02Xh, data %02Xh %02Xh\n", *status, dataBytes[0], dataBytes[1]);
	}
}