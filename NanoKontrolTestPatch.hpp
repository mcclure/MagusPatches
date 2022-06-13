#ifndef __NanoKontrolTest_hpp__
#define __NanoKontrolTest_hpp__

// Works with a Korg nanoKONTROL2 (assumes port 0 channel 0)
// Does nothing, but sprays MIDI messages that make the buttons light up.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"

#define LIGHTS 30

const int lightCC[LIGHTS] = {71, 70, 69, 68, 67, 66, 65, 64, 55, 54, 53, 52, 51, 50, 49, 48, 39, 38, 37, 36, 35, 34, 33, 32, 45, 41, 42, 44, 43, 46};
#define TRIGGER_PERIOD 0.125
#define FLIER_COUNT 4
#define SPECIAL_COUNT 6
#define NORMAL_COUNT (LIGHTS-SPECIAL_COUNT)
#define BZERO(field) memset(field, 0, sizeof(field))

class NanoKontrolTestPatch : public Patch {
private:
  float accum;
  unsigned int fliers[FLIER_COUNT];
  unsigned int fliersActiveCount;
  unsigned int specials[SPECIAL_COUNT];
  unsigned int specialsActiveCount;
  bool lightOn[LIGHTS];
  unsigned int x;
public:
  NanoKontrolTestPatch(){
    accum = 0;
    fliersActiveCount=0;
    specialsActiveCount=0;
    BZERO(fliers); BZERO(specials);
    BZERO(lightOn);
    x = 0;

    char scratch[5] = {'L', 0,0, '>',0};
    for(int c = 0; c < 16; c++) {
      scratch[1] = '0' + (c/10);
      scratch[2] = '0' + (c%10);
      registerParameter((PatchParameterId)c, scratch);
    }
  }

  ~NanoKontrolTestPatch(){
  }

  void processMidi(MidiMessage msg){
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void processAudio(AudioBuffer& buffer){
    float timeStep = ((float)getBlockSize()/getSampleRate());
    accum += timeStep;

    bool lightOnWas[LIGHTS];
    memcpy(lightOnWas, lightOn, sizeof(lightOnWas));
    BZERO(lightOn);
    if (accum > TRIGGER_PERIOD) {
      accum = 0;
      unsigned int specialsActiveCountWas = specialsActiveCount;
      unsigned int completeCount = 0;
      for(int c = 0; c < specialsActiveCountWas; c++) {
        unsigned int &flier = specials[c];
        lightOn[flier+NORMAL_COUNT] = true;

        flier++;
        if (flier >= SPECIAL_COUNT)
          completeCount++;
      }
      if (completeCount) {
        specialsActiveCount -= completeCount;
        if (specialsActiveCount>0) {
          for(int c = 0; c < specialsActiveCount; c++)
            specials[c] = specials[c+completeCount];
        }
      }

      int fliersActiveCountWas = fliersActiveCount;
      for(int c = 0; c < fliersActiveCountWas; c++) {
        unsigned int &flier = fliers[c];
        lightOn[flier] = true;

        flier++;
        if (flier >= NORMAL_COUNT) {
          flier = 0;
          if (fliersActiveCount < FLIER_COUNT && c == fliersActiveCount-1)
            fliersActiveCount++;
          if (specialsActiveCount < SPECIAL_COUNT) {
            specials[specialsActiveCount] = 0;
            specialsActiveCount++;
          }
        }
      }
x++;
      for(int c = 0; c < LIGHTS; c++) {
        //if (lightOn[c] != lightOnWas[c]) {
          MidiMessage msg(0x0B, CONTROL_CHANGE, lightCC[c], x%2?0:127);
          sendMidi(msg);
        //}
        if (c < 16)
          setParameterValue((PatchParameterId)c, x%2?1.0:0.0);
      }
    }

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
};

#endif   // __NanoKontrolTest_hpp__
