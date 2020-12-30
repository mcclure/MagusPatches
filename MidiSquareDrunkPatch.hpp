#ifndef __MidiSquareDrunk_hpp__
#define __MidiSquareDrunk_hpp__

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

class MidiSquareDrunkPatch : public MidiPatchBase {
protected:
  PackedPhase midiPhase[MIDI_MAXDOWN]; // Stack of notes down
  int phaseSelected;
public:
  MidiSquareDrunkPatch() : MidiPatchBase() {
    registerParameter(PARAMETER_A, "Amp");
    setParameterValue(PARAMETER_A, 0.5);
    phaseSelected = 0;
  }

  ~MidiSquareDrunkPatch(){
  }

  void nextNote() {
    phaseSelected++;
    if (phaseSelected > downCount)
      phaseSelected = 0;
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
    if (at >= phaseSelected) {
      phaseSelected--;
      nextNote();
    }
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
    for(int d = 0; d < size; d++) {
      if (downCount) {
        PackedPhase &phase = midiPhase[phaseSelected];
        leftData[d] += phase.high ? 1.0f : -1.0f;
        phase.phase += PHASE_RADIX;
        if (phase.phase > phase.max) {
          phase.phase -= phase.max;
          phase.high = !phase.high;
          if (!phase.high) {
            nextNote();
          }
        }
      } else {
        leftData[d] = 0;
      }
    }
    for(int d = 0; d < size; d++) {
      rightData[d] = leftData[d] = CLAMP(leftData[d]*amp);
    }
  }
};

#endif   // __MidiSquareDrunk_hpp__
