#ifndef __Saw4Patch_hpp__
#define __Saw4Patch_hpp__

// 4 saws, each with its own detune and phase-offset controls.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
//#include "VoltsPerOctave.h"

class Saw4Patch : public Patch {
private:
  uint8_t midinote;
  PatchParameterId semitoneParam[4];
  PatchParameterId microtoneParam[4];
  PatchParameterId phaseOffsetParam[4];
  PatchParameterId waveParam[4];
  PatchParameterId baseParam;
  PatchParameterId overdriveParam;

  double phase[4];

  // Parameters are passed out (TOP ROW THEN BOTTOM ROW) horizontally
  // But we want them to be passed out in 2x2 blocks of 4, left to right
  PatchParameterId patchForSlot(int i) {
    return PatchParameterId((i & 1) | ( (i & 2) << 2 ) | ( (i & 12) >> 1 ));
  }
public:
  // Add a block of 4 parameters for a single oscillator
  void add4Params(PatchParameterId base, const char *name, int id) {
    char scratch[16];
    PatchParameterId param;

    param = patchForSlot(base + 0);
    strncpy(scratch, "Semitone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    microtoneParam[id] = param;

    param = patchForSlot(base + 1);
    strncpy(scratch, "Microtone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    semitoneParam[id] = param;

    param = patchForSlot(base + 2);
    strncpy(scratch, "Phase ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    phaseOffsetParam[id] = param;

    param = patchForSlot(base + 3);
    strncpy(scratch, "Wave ", 16); strncat(scratch, name, 16); strncat(scratch, ">", 16);
    registerParameter(param, scratch);
    waveParam[id] = param;
  }
  Saw4Patch(){        
    add4Params(PARAMETER_A, "A", 0);
    add4Params((PatchParameterId)(PARAMETER_A+4), "B", 1);
    add4Params((PatchParameterId)(PARAMETER_A+8), "C", 2);
    add4Params((PatchParameterId)(PARAMETER_A+12), "D", 3);

    baseParam = (PatchParameterId)(PARAMETER_A+16);
    registerParameter(baseParam, "Base Semi");
    overdriveParam = (PatchParameterId)(PARAMETER_A+17);
    registerParameter(overdriveParam, "Overdrive");

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

    // Parameters for entire pass
    float base = midinote + getParameterValue(baseParam);
    float overdrive = getParameterValue(overdriveParam)/4.0f;
    float sampleRateDiv2 = getSampleRate() / 2.0f;

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    float semitone[4];
    float microtone[4];
    float phaseOffset[4];
    float waveStep[4];

    for(int w = 0; w < 4; w++) {
      semitone[w] = getParameterValue(semitoneParam[w]);
      microtone[w] = getParameterValue(microtoneParam[w]);
      phaseOffset[w] = getParameterValue(phaseOffsetParam[w]);
      // Haven't checked this math
      float playTone = (base + semitone[w] + microtone[w]/128.0f - 69)/12.0f; // Power-2 offset from A440
      waveStep[w] = (440*exp2(playTone)) / sampleRateDiv2;
    }

    for(int i = 0; i < size; i++ ) {
      float sample = 0;
      for(int w = 0; w < 4; w++) {
        // Parameters for single wave evaluation

        // Update phase and wrap into -1..1 range
        phase[w] = fmodf( (phase[w] + waveStep[w]), 1.0 );

        // Wave value
        float value = fmodf(phase[w] + phaseOffset[w], 1.0f); //phase[w]; //fmod( phase[w] + phaseOffset[w], 1.0 );

        // Add wave to sample
        sample += value*overdrive;
      }
      // Clamp sample to -1..1
      sample = max(-1.0f, (min(1.0f, sample)));

      // Write sample
      leftData[i] = sample;
      rightData[i] = sample;
    }

    for(int w = 0; w < 4; w++) {
      setParameterValue(waveParam[w], phase[w]);
    }
  }
};

#endif   // __Saw4Patch_hpp__
