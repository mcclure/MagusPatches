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
public:
  // Add a block of 4 parameters for a single oscillator
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

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    for(int i = 0; i < size; i++ ) {
      float sample = 0;
      for(int w = 0; w < 4; w++) {
        // Parameters for single wave evaluation
        float semitone = getParameterValue(semitoneParam[w]);
        float microtone = getParameterValue(microtoneParam[w]);
        float phaseOffset = getParameterValue(phaseOffsetParam[w]);
        // Wavelength of note to play (haven't checked this math)
        float playTone = (base + semitone + microtone/128.0f)/12.0f;

        // Update phase and wrap into -1..1 range
        phase[w] = fmod( (phase[w] + 2.0f/44100.f * /*exp2f(playTone)*/), 1.0 );

        // Wave value
        float value = phase[w];
        setParameterValue(waveParam[w], value);

        // Add wave to sample
        sample += value*overdrive;
      }
      // Clamp sample to -1..1
      sample = max(-1, (min(1, sample)));

      // Write sample
      leftData[i] = sample;
      rightData[i] = sample;
    }
  }
};

#endif   // __Saw4Patch_hpp__
