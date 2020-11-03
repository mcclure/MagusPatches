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

# Development helpers

## Test patches

### ScreenSaver

I wrote this patch to test the screen. It just draws random numbers in random positions.

## MagusSim

`MagusSim/MakeMagusSim.py` is a script which will take any OWL Patch.hpp file and convert it into a standalone program. The standalone program will (by default) run for one second and print the PCM output to stdout, but there is an option to print human-readable floats.

For instructions on using MakeMagusSim.py, run `./MagusSim/MakeMagusSim.py --help`. Note if your input hpp file includes other hpp files, you must include an `-i` argument for each.

For instructions on using the standalone program, run `./Saw4Patch --help` (or whatever the name is).

Some parts of the OWL API are not supported inside the simulator. You can mark sections of code that don't need to run in the simulator (like MIDI) with `#ifndef OWL_SIMULATOR` (or `#ifdef OWL_MAGUS` or `#ifdef USE_SCREEN`).

Here is an example of using MagusSim:

    ./MagusSim/MakeMagusSim.py Saw4Patch.hpp
    ./Saw4Patch > saw4.raw

You can then open the .raw file using Audacity or Amadeus (for mac) as floating-point stereo, little endian (or the endianness of your machine).
