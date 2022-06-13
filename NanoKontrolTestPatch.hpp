#ifndef __NanoKontrolTest_hpp__
#define __NanoKontrolTest_hpp__

// Works with a Korg nanoKONTROL2 (assumes port 0 channel 0)
// Does nothing, but sprays MIDI messages that make the buttons light up.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"

#define LIGHTS 30

const int lightCC[LIGHTS] = {71, 70, 69, 68, 67, 66, 65, 64, 55, 54, 53, 52, 51, 50, 49, 48, 39, 38, 37, 36, 35, 34, 33, 32, 45, 41, 42, 44, 43, 46};
#define TRIGGER_PERIOD 0.125
#define FLIER_COUNT 3
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
    fliersActiveCount=1;
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
    for(unsigned int c = 0; c < LIGHTS; c++)
      lightSet(c, false);
  }

  ~NanoKontrolTestPatch(){
  }

  void processMidi(MidiMessage msg){
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void lightSet(unsigned int light, bool on) {
    MidiMessage msg(0x0B, CONTROL_CHANGE, lightCC[light], on?127:0);
    sendMidi(msg);
  }

  void processAudio(AudioBuffer& buffer){
    float timeStep = ((float)getBlockSize()/getSampleRate());
    accum += timeStep;

    if (accum > TRIGGER_PERIOD) {
      accum = 0;

      bool lightOnWas[LIGHTS];
      memcpy(lightOnWas, lightOn, sizeof(lightOnWas));
      BZERO(lightOn);

      unsigned int specialsActiveCountWas = specialsActiveCount;
      unsigned int completeCount = 0;
      for(unsigned int c = 0; c < specialsActiveCountWas; c++) {
        unsigned int &flier = specials[c];
        lightOn[flier+NORMAL_COUNT] = true;

        flier++;
        if (flier >= SPECIAL_COUNT)
          completeCount++;
      }
      if (completeCount) {
        specialsActiveCount -= completeCount;
        if (specialsActiveCount>0) {
          for(unsigned int c = 0; c < specialsActiveCount; c++)
            specials[c] = specials[c+completeCount];
        }
      }

      unsigned int fliersActiveCountWas = fliersActiveCount;
      for(unsigned int c = 0; c < fliersActiveCountWas; c++) {
        unsigned int &flier = fliers[c];
        int speed = (1 << c);
        int effectiveFlier = flier / speed;
        lightOn[effectiveFlier] = true;

        flier++;
        if (effectiveFlier != 0 && effectiveFlier%8 == 0 && flier%speed==0) {
          if (specialsActiveCount < SPECIAL_COUNT && 
              (specialsActiveCount==0 || specials[specialsActiveCount-1] != 0)) {
            specials[specialsActiveCount] = 0;
            specialsActiveCount++;
          }
        }
        if (effectiveFlier >= NORMAL_COUNT) {
          flier = 0;
          if (fliersActiveCount < FLIER_COUNT && c == fliersActiveCountWas-1)
            fliersActiveCount++;
        }
      }

      for(unsigned int c = 0; c < LIGHTS; c++) {
        if (lightOn[c] != lightOnWas[c]) {
          lightSet(c, lightOn[c]);
        }
        if (c < 16)
          setParameterValue((PatchParameterId)c, lightOn[c]?1.0:0.0);
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
