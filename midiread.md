# MIDIread

## MIDI file structure

Each MIDI file contains several *chunks*.  Each chunk is either a *header chunk* or a *track chunk*.

Each track chunk contains several *events*.  Each event is either a *MIDI event*, *sysex event*, or *meta-event*.

## Features to possibly implement

* Tidy up a MIDI file, to allow optional changes to it
	* Decrease the resolution to the lowest possible for that song, down to 24 PPQN
	* Add markers (named "In" and "Out") a bar before any part starts and a bar (or perhaps 4, to capture any tail in the sound) after it ends -- this would be useful for working with a transport control with skip-to-marker functions
	* Explode drum track to one track per pitch -- this would work around Reaper's explosion bug entirely
* Convert a raw sysex message into a MIDI file containing it