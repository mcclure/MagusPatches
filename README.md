Patches for the [Rebel Technology OWL](https://www.rebeltech.org/) by Andi McClure <<andi.m.mcclure@gmail.com>>. Patches were tested with the [Magus](https://www.rebeltech.org/product/magus/) and some of them use the screen. There is also a PC-side test harness.

Build instructions in [run.txt](run.txt). Or you can paste the .hpp files into [the RebelTech patches website](https://www.rebeltech.org/patch-library).

Licenses are included in their individual files but are mostly CC0/public domain. If you redistribute these programs as source code, it would be *polite* to retain the attribution even though (for the CC0 files) you are not legally required to do so.

# MagusSim

`MagusSim/MakeMagusSim.py` is a script which will take any OWL Patch.hpp file and convert it into a standalone program. The standalone program will (by default) run for one second and print the PCM output to stdout, but there is an option to print human-readable floats.

For instructions on using MakeMagusSim.py, run `./MagusSim/MakeMagusSim.py --help`. Note if your input hpp file includes other hpp files, you must include an `-i` argument for each.

For instructions on using the standalone program, run `./Saw4Patch --help` (or whatever the name is).

Some parts of the OWL API are not supported inside the simulator. You can mark sections of code that don't need to run in the simulator (like MIDI) with `#ifndef OWL_SIMULATOR`.

Here is an example of using MagusSim:

    ./MagusSim/MakeMagusSim.py Saw4Patch.hpp
    ./Saw4Patch > saw4.raw

You can then open the .raw file using Audacity or Amadeus (for mac) as floating-point stereo, little endian (or the endianness of your machine).
