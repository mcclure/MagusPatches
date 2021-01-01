#ifndef __PureDelayPatch_hpp__
#define __PureDelayPatch_hpp__

// MIDI-controllable delay, no feedback. Monitor in left channel.
// Author Andi McClure. License https://opensource.org/licenses/MIT

#include "OpenWareMidiControl.h"
#include "math.h"
#include "support/midi.h"
#include "support/patchForSlot.h"
//#include "VoltsPerOctave.h"

#define BUFSIZE (44100*2+1)

#define BASEDELAY     (patchForSlot(PARAMETER_A))
#define MICRODELAY    (patchForSlot(PARAMETER_A+4))
#define MIDIDELAY     (patchForSlot(PARAMETER_A+8))
#define INPUTLOUD     (patchForSlot(PARAMETER_A+12))
#define OUTPUTLOUD    (patchForSlot(PARAMETER_A+13))
#define MONITORLOUD   (patchForSlot(PARAMETER_A+14))
#define COMONITORLOUD (patchForSlot(PARAMETER_A+15))

class PureDelayPatch : public Patch {
private:
  uint8_t midinote;

  float history[BUFSIZE];
  int writePtr;

public:
  PureDelayPatch() {
    midinote = MIDDLEC_MIDI;
    writePtr = 0;
    memset(history, 0, sizeof(history));
    registerParameter(BASEDELAY, "Base Delay");
    setParameterValue(BASEDELAY, 0.5);
    registerParameter(MICRODELAY, "Micro Delay");
    setParameterValue(MICRODELAY, 0.5);
    registerParameter(MIDIDELAY, "MIDI amplitude");
    setParameterValue(MIDIDELAY, 0.5);
    registerParameter(INPUTLOUD, "Input attenuation");
    setParameterValue(INPUTLOUD, 1.0);
    registerParameter(OUTPUTLOUD, "Output attenuation");
    setParameterValue(OUTPUTLOUD, 1.0);
    registerParameter(MONITORLOUD, "Monitor attenuation");
    setParameterValue(MONITORLOUD, 1.0);
    registerParameter(COMONITORLOUD, "Monitor output mix");
    setParameterValue(COMONITORLOUD, 1.0);
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

#define PCLAMP(x) max(0.0f, (min(1.0f, (x))))

  int backLook() {
    float base  = getParameterValue(BASEDELAY);
    float micro = getParameterValue(MICRODELAY);
    float midi  = getParameterValue(MIDIDELAY);
    float across = PCLAMP(base + (micro-0.5f)/16.0 + midi*(midinote-MIDDLEC_MIDI)/64.0f);

    return across*(BUFSIZE-1);
  }

#define CLAMP(x) max(-1.0f, (min(1.0f, (x))))

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    float inputLoud = getParameterValue(INPUTLOUD);
    float outputLoud = getParameterValue(OUTPUTLOUD);
    float monitorLoud = getParameterValue(MONITORLOUD);
    float comonitorLoud = getParameterValue(COMONITORLOUD);

    for(int i = 0; i < size; i++ ) {
      float sample = CLAMP((leftData[i] + rightData[i])*inputLoud);

      history[writePtr++] = sample;
      writePtr %= BUFSIZE;
    }

    int back = backLook();
    int from = writePtr - back - size + 2*BUFSIZE;
    for(int i = 0; i < size; i++ ) {
      int at = (from+i)%BUFSIZE;
      float sample = history[at]*outputLoud;
      
      leftData[i] = CLAMP(leftData[i]*monitorLoud + sample*comonitorLoud);
      rightData[i] = sample;
    }
  }

#ifdef USE_SCREEN
  void processScreen(ScreenBuffer& screen){ // TODO
  }
#endif

};

#endif   // __PureDelayPatch_hpp__
