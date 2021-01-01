#ifndef __MidiSquare_hpp__
#define __MidiSquare_hpp__

// Midi input is converted to a CV/gate outputs. The notes currently down are displayed.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/
// If you reuse this code preserving credit is appreciated but not legally required.

#include "OpenWareMidiControl.h"
#include "support/midiPatchBase.hpp"
#include "support/midi.h"
#include "basicmaths.h"

#define PHASE_RADIX 16

struct PackedPhase {
  unsigned int high:1;
  unsigned int max:15;
  unsigned int phase:15;
};

class MidiSquarePatch : public MidiPatchBase {
protected:
  PackedPhase midiPhase[MIDI_MAXDOWN]; // Stack of notes down
public:
  MidiSquarePatch() : MidiPatchBase() {
    registerParameter(PARAMETER_A, "Amp");
    setParameterValue(PARAMETER_A, 0.5);
  }

  ~MidiSquarePatch(){
  }

  void startNote(int at, uint8_t midiNote) {
    PackedPhase &phase = midiPhase[at];
    float sampleRateDiv2 = getSampleRate() / 2.0f;
    float period = sampleRateDiv2 / (440.0f*exp2((midiNote-69.0f)/12.0f));
    phase.high = 0;
    phase.max = period*PHASE_RADIX;
    phase.phase = 0;
  }

  void killNote(int at) {
    for(int c = at; c<(int)downCount-1; c++)
      midiPhase[c] = midiPhase[c+1];
  }

  #define CLAMP(x) max(-1.0f, (min(1.0f, (x))))

  void processAudio(AudioBuffer& buffer) { // Create CV/Gate from notes down
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    float amp = getParameterValue(PARAMETER_A)/4.0;

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    // Write samples
    for(int c = 0; c < downCount; c++) {
      PackedPhase &phase = midiPhase[c];
      for(int d = 0; d < size; d++) {
        rightData[d] += phase.high ? 1.0f : -1.0f;
        phase.phase += PHASE_RADIX;
        if (phase.phase > phase.max) {
          phase.phase -= phase.max;
          phase.high = !phase.high;
        }
      }
    }
#if PATCH_STEREO
    for(int d = 0; d < size; d++) {
      leftData[d] = rightData[d] = CLAMP(leftData[d]*amp);
    }
#else
    for(int d = 0; d < size; d++) {
      rightData[d] = CLAMP(rightData[d]*amp);
    }
    // Write samples
    float temp = isDown ? 1.0f : 0.0f;
    {
      int c = 0;
      for(; needRetrig && c < size; c++, needRetrig--) // First write needRetrig 0s
        leftData[c] = 0.0f;
      for(; c < size; c++) // Then if down write 1s
        leftData[c] = temp;
    }
#endif
  }
};

#endif   // __MidiSquare_hpp__
