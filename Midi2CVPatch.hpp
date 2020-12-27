#ifndef __Midi2CV_hpp__
#define __Midi2CV_hpp__

// Midi input is converted to a CV/gate outputs. The notes currently down are displayed.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/
// If you reuse this code preserving credit is appreciated but not legally required.

#include "OpenWareMidiControl.h"
#include "support/midiPatchBase.hpp"
#include "support/midi.h"
#include "VoltsPerOctave.h"
#include "basicmaths.h"

class Midi2CVPatch : public MidiPatchBase {
public:
  Midi2CVPatch() : MidiPatchBase() {
  }

  ~Midi2CVPatch(){
  }

  void processAudio(AudioBuffer& buffer) { // Create CV/Gate from notes down
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    // Write samples
    float temp = isDown ? 1.0f : 0.0f;
    {
      int c = 0;
      for(; needRetrig && c < size; c++, needRetrig--) // First write needRetrig 0s
        leftData[c] = 0.0f;
      for(; c < size; c++) // Then if down write 1s
        leftData[c] = temp;
    }

    temp = (lastMidi - 33) / (12.0f * 5.0f); // We can output notes A1 to G#6
    for(int c = 0; c < size; c++)
      rightData[c] = temp;
  }
};

#endif   // __Midi2CV_hpp__
