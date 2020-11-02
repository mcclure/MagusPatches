#ifndef __support_midi_hpp__
#define __support_midi_hpp__

#define MIDDLEC_MIDI 60
#define MIDDLEC_OCTAVE 4

enum {
  MidiCodeNoteOff = 0x80,
  MidiCodeNoteOn  = 0x90
} MidiCodes;

#endif // __support_midi_hpp__