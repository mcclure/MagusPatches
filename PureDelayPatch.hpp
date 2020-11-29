#ifndef __PureDelayPatch_hpp__
#define __PureDelayPatch_hpp__

// 4 saws, each with its own detune and phase-offset controls.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
#include "math.h"
#include "support/midi.h"
//#include "VoltsPerOctave.h"

#define BUFSIZE 44100*4

class PureDelayPatch : public Patch {
private:
  uint8_t midinote;

  float history[BUFSIZE];
  int writePtr;

  // Parameters are passed out (TOP ROW THEN BOTTOM ROW) horizontally
  // But we want them to be passed out in 2x2 blocks of 4, left to right
  PatchParameterId patchForSlot(int i) {
    return PatchParameterId((i & 1) | ( (i & 2) << 2 ) | ( (i & 12) >> 1 ));
  }
public:
  PureDelayPatch() {
    midinote = MIDDLEC_MIDI;
    writePtr = 0;
    memset(history, 0, sizeof(history));
  }

  ~PureDelayPatch(){
  }

#ifndef OWL_SIMULATOR
  void processMidi(MidiMessage msg){
    switch (msg.getStatus()) {
      // Key on
      case MidiCodeNoteOn:
        midinote = msg.getNote();
        break;
      default:break;
    }
  }
#endif

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  int backLook() {
    return 44100;
  }

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    for(int i = 0; i < size; i++ ) {
      float sample = max(-1.0f, (min(1.0f, leftData[i] + rightData[i])));

      history[writePtr++] = sample;
      writePtr %= BUFSIZE;
    }

    int back = backLook();
    int from = writePtr - back - size + BUFSIZE;
    for(int i = 0; i < size; i++ ) {
      int at = (from+i)%BUFSIZE;
      float sample = history[at];
      
      leftData[i] = sample;
      rightData[i] = sample;
    }
  }

#ifdef USE_SCREEN
  void processScreen(ScreenBuffer& screen){ // Print notes-down stack
  }
#endif

};

#endif   // __PureDelayPatch_hpp__
