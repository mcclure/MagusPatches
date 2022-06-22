Patches for the [Rebel Technology OWL](https://www.rebeltech.org/) by Andi McClure <<andi.m.mcclure@gmail.com>>. Patches were tested with the [Magus](https://www.rebeltech.org/product/magus/) and some of them use the screen. There is also a PC-side test harness.

Build instructions in [run.txt](run.txt). Or you can paste the .hpp files into [the RebelTech patches website](https://www.rebeltech.org/patch-library).

Licenses are included in their individual files but are mostly CC0/public domain. If you redistribute these programs as source code, it would be *polite* to retain the attribution even though (for the CC0 files) you are not legally required to do so.

# Patches

## Midi2CV

This converts incoming MIDI to CV/Gate. If you hold multiple keys it will nicely play only the last one and retrigger when keys are lifted.

The Gate is on the left audio output and the CV is on the right audio output. You must turn volume up to 100% or this will not work right.

Also on RebelTech [here](https://www.rebeltech.org/patch-library/patch/Midi2CV).

## Saw4

This is a 4-oscillator synth voice with independent detune on each voice. Set the voices at slightly different microtone detunes and turn up "overdrive" for nice growls. Also on RebelTech [here](https://www.rebeltech.org/patch-library/patch/AndiSaw4).

## Silence

Does nothing at all. I load this as my "patch 1" so that when the device first boots up, the CPU load and power draw are as low as possible. Also on RebelTech [here](https://www.rebeltech.org/patch-library/patch/Silence).

## MidiMonitor

Displays incoming MIDI messages to the screen. Useful as a development tool or for debugging MIDI. Requires a device with a screen (Magus or Genius).

Line format: meaningless incrementing number; MIDI message as 8 nibbles of hex; "c" followed by channel number; and then exactly one of:
	* (For normal note) ON or OFF, note value
	* (Control Change) "CC", hex of controller number, hex of controller value
	* (Pitchbend) "BD", + or -, decimal pitchbend value
	* (Any other message) "stat", hex of status code

## NanoKontrolSeq

Interactive CV sequencer based on the Kork NanoKontrol2 MIDI controller. Currently designed for use with the Magus, and has not been tested on other OpenWare devices.

To use, obtain a NanoKontrol2 and use the Korg Kontrol software for desktop PC to configure it with the file [UserFiles/OpenwareNanoKontrol2-Safe.nktrl2_data](UserFiles/OpenwareNanoKontrol2-Safe.nktrl2_data). If you then plug in the NanoKontrol2 to the Magus "USB HOST" port and run this patch, the buttons will take on the following functions:

	* "R" buttons 1-8: Selects one of 8 "notes" in the current sequence. Should light up depending on what note is playing.
	* Sliders 1-8: Sets the value for 8 "lanes" (CV output values) 0-8V.
	* Knobs 1-8: Finetunes the value for the 8 "lanes"
	* Play: Cycle through the 8 notes. If already playing, will pause at the current note.
	* Stop: Stop playing and reset to note 1.
	* << and >> : Step forward or back one note.
	* "S" buttons 1-8: Special behavior in "lockdown" or "performance" mode (see below)
	* "Cycle": This is the "Shift" button; it changes the meaning of certain other buttons:

		* Cycle+R: Set the length of the pattern used by "Play" and <</>>
		* Cycle+>>: When this is held down, lane 8 becomes a BPM control. (EXPERIMENTAL:) Also lane 1 and 2 control the relative rate of a "click track" that plays in the L and R audio output channels (in range "play 16 times per note" to "play once every 16 notes".
		* Cycle+<<: Sequencer enters/leaves a "lockdown" mode where sliders and knobs have no effect. To change the value of a lane, hold the "S" button next to it. "<<" button will light up.
		* Cycle+STOP: (EXPERIMENTAL:) If sequencer is already in "lockdown" mode, enters a "performance" mode where sliders and knobs have no effect normally, and when the "S" button for a lane is held down the last value of that slider/knob takes precedence over the current note. The difference between "lockdown" and "performance" mode is that in lockdown mode "S"ending a slider value will overwrite the current note in the sequence, but in performance mode it will only "play" (but not be written).

# Development helpers

## Test patches

### ScreenSaver

I wrote this patch to test the screen. It just draws random numbers in random positions.

### NanoKontrolTestPatch

If a Korg NanoKontrol2 is attached, flickers all the lights in an ascending pattern. This assumes a configuration set in the KORG KONTROL app where LEDs are set "externally", but it uses the default configuration rather than the "OpenwareNanoKontrol2-SAFE" configuration.

## MagusSim

`MagusSim/MakeMagusSim.py` is a script which will take any OWL Patch.hpp file and convert it into a standalone program. The standalone program will (by default) run for one second and print the PCM output to stdout, but there is an option to print human-readable floats.

For instructions on using MakeMagusSim.py, run `./MagusSim/MakeMagusSim.py --help`. Note if your input hpp file includes other hpp files, you must include an `-i` argument for each.

For instructions on using the standalone program, run `./Saw4Patch --help` (or whatever the name is).

Some parts of the OWL API are not supported inside the simulator. You can mark sections of code that don't need to run in the simulator (like MIDI or screen code) with `#ifndef OWL_SIMULATOR`.

Here is an example of using MagusSim:

    ./MagusSim/MakeMagusSim.py Saw4Patch.hpp
    ./Saw4Patch > saw4.raw

You can then open the .raw file using Audacity or Amadeus (for mac) as floating-point stereo, little endian (or the endianness of your machine).
