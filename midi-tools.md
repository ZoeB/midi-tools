# MIDI tools

## midi.pdf

I've now rewritten the repo's history to undo commit ```3ae91873```, removing ```midi.pdf```.  This file is "The Complete MIDI 1.0 Detailed Specification, Incorporating all Recommended Practices, document version 96.1, third edition", with an MD5 beginning with ```44f3f11c```.  This file was ridiculously larger than the rest of the repo, and more to the point, the MIDI Manufacturers Association don't seem to want it to be public, preferring you to have to register for a free account first at [midi.org](https://www.midi.org).

## MIDI file structure

Each MIDI file contains several *chunks*.  Each chunk is either a *header chunk* or a *track chunk*.

Each track chunk contains several *events*.  Each event is either a *MIDI event*, *sysex event*, or *meta-event*.

## Bugs to fix

* Verify pitchbend is being displayed accurately.  Reaper seems to disagree.

## Features to possibly implement

* Show the pitchbend amount in cents, based on what the range is set to
* Show all the timings in absolute bars (with 3 decimal places) or HH:MM:SS:FF, not relative raw ticks
* Allow Real-Time events to occur inside unrelated events' data bytes (P37).  This will allow the MIDI event handling function to work on both MIDI Files and live MIDI communication.
* Tidy up a MIDI file, to allow optional changes to it
	* Decrease the resolution to the lowest possible for that song, down to 24 PPQN
	* Add markers (named "In" and "Out") a bar before any part starts and a bar (or perhaps 4, to capture any tail in the sound) after it ends -- this would be useful for working with a transport control with skip-to-marker functions
	* Explode drum track to one track per pitch -- this would work around Reaper's explosion bug
* Create ```syxtomid.c```: Convert one or more .syx files each containing a single sysex message into a MIDI file containing them all (format 0 if from one .syx file, format 2 if from multiple .syx files)