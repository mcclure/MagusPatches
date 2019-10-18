#ifndef __Saw4Patch_hpp__
#define __Saw4Patch_hpp__

// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "StompBox.h"
#include "VoltsPerOctave.h"

class Saw4Patch : public Patch {
private:
  uint8_t midinote;
  PatchParameterId semitoneParam[4];
  PatchParameterId microtoneParam[4];
  PatchParameterId phaseOffsetParam[4];
  PatchParameterId waveParam[4];

  double phase[4];
public:
  void add4Params(PatchParameterId base, const char *name, int id) {
    char scratch[16];
    PatchParameterId param;

    param = (PatchParameterId)(base + 0);
    strncpy(scratch, "Semitone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    microtoneParam[id] = param;

    param = (PatchParameterId)(base + 1);
    strncpy(scratch, "Microtone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    semitoneParam[id] = param;

    param = (PatchParameterId)(base + 2);
    strncpy(scratch, "Phase ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    phaseOffsetParam[id] = param;

    param = (PatchParameterId)(base + 3);
    strncpy(scratch, "Wave ", 16); strncat(scratch, name, 16); strncat(scratch, ">", 16);
    registerParameter(param, scratch);
    waveParam[id] = param;
  }
  Saw4Patch(){
    // OSC
    registerParameter(PARAMETER_A, "Base Semi");
    registerParameter(PARAMETER_B, "Overdrive");
    
    add4Params(PARAMETER_AA, "A", 0);
    add4Params(PARAMETER_BA, "B", 1);
    add4Params(PARAMETER_CA, "C", 2);
    add4Params(PARAMETER_DA, "D", 3);

    memset(phase, 0, sizeof(phase));

    midinote = 69;
  }

  ~Saw4Patch(){
  }

  void noteOn(uint8_t note, uint16_t velocity, uint16_t samples){
    midinote = note;
  }

  void noteOff(uint8_t note, uint16_t samples){
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // synth voice
    float base = midinote + getParameterValue(PARAMETER_A);
    float overdrive = getParameterValue(PARAMETER_B)/4.0f;

    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();


    for(int i = 0; i < size; i++ ) {
      float &leftSample = leftData[i];
      leftSample = 0;
      for(int w = 0; w < 4; w++) {
        float semitone = getParameterValue(semitoneParam[w]);
        float microtone = getParameterValue(microtoneParam[w]);
        float phaseOffset = getParameterValue(phaseOffsetParam[w]);
        phase[w] = phase[w] + 0.01;
        float value = phase[w];

        setParameterValue(waveParam[w], value);

        leftSample += value*getParameterValue(PARAMETER_B);
      }
      leftSample = max(-1, (min(1, leftSample)));
      rightData[i] = leftSample;
    }
  }
};

#endif   // __KickBoxPatch_hpp__
