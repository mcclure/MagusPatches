#ifndef __Saw4Patch_hpp__
#define __Saw4Patch_hpp__

// 4 saws, each with its own detune and phase-offset controls.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
#include "math.h"
#include "support/patchForSlot.h"
//#include "VoltsPerOctave.h"

class Saw4Patch : public Patch {
private:
  uint8_t midinote;
  PatchParameterId semitoneParam[4];
  PatchParameterId microtoneParam[4];
  PatchParameterId phaseOffsetParam[4];
  PatchParameterId waveParam[4];
  PatchParameterId mixParam[2];
  PatchParameterId baseParam;
  PatchParameterId overdriveParam;

  double phase[4];

public:
  // Add a block of 4 parameters for a single oscillator
  void add4Params(PatchParameterId base, const char *name, int id) {
    char scratch[16];
    PatchParameterId param;

    param = patchForSlot(base + 0);
    strncpy(scratch, "Semitone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    semitoneParam[id] = param;
    setParameterValue(param, 0.5);

    param = patchForSlot(base + 1);
    strncpy(scratch, "Microtone ", 16); strncat(scratch, name, 16);
    registerParameter(param, scratch);
    microtoneParam[id] = param;
    setParameterValue(param, 0.5);

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
    setParameterValue(baseParam, 0.5);
    overdriveParam = (PatchParameterId)(PARAMETER_A+17);
    registerParameter(overdriveParam, "Overdrive");

    for(int w = 0; w < 2; w++) {
      const char *label = w == 0 ? "Mix- BCD" : "Mix- CD";
      PatchParameterId param = (PatchParameterId)(PARAMETER_A + 18 + w);
      registerParameter(param, label);
      mixParam[w] = param;
      setParameterValue(param, 1.0);
    }

    memset(phase, 0, sizeof(phase));

    midinote = 64;
  }

  ~Saw4Patch(){
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

  inline static float mod11(float f) {
    return fmodf(f+1, 2)-1;
  }

  inline static float offsetToSemitones(float f) {
    return roundf((f-0.5)*64);
  }

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Parameters for entire pass
    float base = midinote + offsetToSemitones(getParameterValue(baseParam));
    float overdrive = (1 + getParameterValue(overdriveParam)*32.0f)/4.0f;
    float sampleRateDiv2 = getSampleRate() / 2.0f;

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    float semitone[4];
    float microtone[4];
    float phaseOffset[4];
    float waveStep[4];
    float mix[4];

    float mixBCD = getParameterValue(mixParam[0]);
    float mixCD =  getParameterValue(mixParam[1]);

    for(int w = 0; w < 4; w++) {
      semitone[w] = offsetToSemitones(getParameterValue(semitoneParam[w]));
      microtone[w] = getParameterValue(microtoneParam[w]) - 0.5;
      phaseOffset[w] = getParameterValue(phaseOffsetParam[w]);

      mix[w] = 1;
      if (w>0) mix[w] *= mixBCD;
      if (w>1) mix[w] *= mixCD;

      float playTone = (base + semitone[w] + microtone[w] - 69)/12.0f; // Power-2 offset from A440 (69)
      waveStep[w] = (440*exp2(playTone)) / sampleRateDiv2;
    }

    for(int i = 0; i < size; i++ ) {
      float sample = 0;
      for(int w = 0; w < 4; w++) {
        // Parameters for single wave evaluation

        // Update phase and wrap into -1..1 range
        phase[w] = mod11(phase[w] + waveStep[w]);

        // Wave value
        float value = mod11(phase[w] + phaseOffset[w]); //phase[w]; //fmod( phase[w] + phaseOffset[w], 1.0 );

        // Add wave to sample
        sample += value*overdrive*mix[w];
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
