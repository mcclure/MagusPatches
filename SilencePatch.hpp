#ifndef __Silence_hpp__
#define __Silence_hpp__

// Does nothing. I load this as patch 1 so that the CPU/power load is low on bootup.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"

class SilencePatch : public Patch {
private:
public:
  SilencePatch(){
  }

  ~SilencePatch(){
  }

  void processMidi(MidiMessage msg){
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    // Write sample
    memset(leftData,  0, size*sizeof(float));
    memset(rightData, 0, size*sizeof(float));
  }

  void processScreen(ScreenBuffer& screen){
  }
};

#endif   // __Saw4Patch_hpp__
