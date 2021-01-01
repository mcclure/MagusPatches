#ifndef __Midi2CVTriplet_hpp__
#define __Midi2CVTriplet_hpp__

// Midi input is converted to a CV/gate outputs. The notes currently down are displayed.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/
// If you reuse this code preserving credit is appreciated but not legally required.

#include "OpenWareMidiControl.h"
#include "support/midiPatchBase.hpp"
#include "support/midi.h"
#include "support/patchForSlot.h"
#include "basicmaths.h"

#define MIDI_OUTS 3
#define PARAM_BASE 0

// Midi2CV but uses 3 bottom-area outlets. For the Chainsaw
class Midi2CVTripletPatch : public MidiPatchBase {
public:
  int midiAssign[MIDI_MAXDOWN]; // Stack of notes down

  Midi2CVTripletPatch() : MidiPatchBase() {
    char scratch[16];
    PatchParameterId param;

    param = patchForSlot(PARAM_BASE);
    registerParameter(param, "Trig>");
    setParameterValue(param, 0);

    for(int c = 0; c < MIDI_OUTS; c++) {
      param = patchForSlot(PARAM_BASE + 1 + c);
      strncpy(scratch, "CV", 16); scratch[2] = '0' + c; scratch[3] = '>'; scratch[4] = '\0';
      registerParameter(param, scratch);
      setParameterValue(param, 0.5);
    }
  }

  ~Midi2CVTripletPatch(){
  }

  void startNote(int at, uint8_t midiNote) {
    int safe = -1;
    for(int check = 0; safe < 0 && check < MIDI_OUTS;) { // Iterate over possible values
      safe = check;
      for(int c = 0; c < at; c++) { // For each existing note, check if it's already in the array
        if (midiAssign[c] == check) {
          safe = -1;
          check++;
          break;
        }
      }
    }
    if (safe < 0) { // Didn't find anything. Steal a note.
      for(int c = 0; c < at; c++) {
        if (midiAssign[c] >= 0) {
          safe = midiAssign[c];
          midiAssign[c] = -1;
          break;
        }
      }
    }
    midiAssign[at] = safe;
  }

  void killNote(int at) {
    int reassign = midiAssign[at];
    for(int c = at; c<(int)downCount-1; c++)
      midiAssign[c] = midiAssign[c+1];
    for(int c = at; c>= 0; c--) {
      if (midiAssign[c] == -1) {
        midiAssign[c] = reassign;
        break;
      }
    }
  }

  void processAudio(AudioBuffer& buffer) { // Create CV/Gate from notes down
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    memset(leftData, 0, size*sizeof(float));
    memset(rightData, 0, size*sizeof(float));

    int assign = lastMidi;

    PatchParameterId param = patchForSlot(PARAM_BASE);
    if (downCount) {
      if (needRetrig > 0) {
        needRetrig = needRetrig < size ? 0 : needRetrig - size;
        setParameterValue(param, 0.0f);
      } else {
        setParameterValue(param, 1.0f);
      }
    } else {
      setParameterValue(param, 0.0f);
    }

    for(int c = 0; c < MIDI_OUTS; c++) {
      for(int d = 0; d < downCount; d++) {
        if (midiAssign[d] == c) {
          assign = midiDown[d];
          break;
        }
      }
      param = patchForSlot(PARAM_BASE + 1 + c);
      float value = (assign - 33) / (12.0f * 5.0f); // We can output notes A1 to G#6
      setParameterValue(param, value);
    }
  }

  #ifdef USE_SCREEN
  // NOTE: This is a cutpaste of midiBasePatch implementation, I don't like that, it could be abstracted in principle
  void processScreen(ScreenBuffer& screen){ // Print notes-down stack
//debugMessage("Note count", downCount);
    int height = screen.getHeight();
    bool first = true;
    screen.clear();
    screen.print(0,8,""); // FIXME magic number?
    if (downCount > 0) {
      for(int c = 0; c < downCount; c++) {
        if (!first) { // Print space between values
          screen.setTextColour(WHITE, BLACK);
          screen.print(" ");
        } else {
          first = false;
        }
        if (midiAssign[c] >= 0) // Highlight all currently selected notes by inverting
          screen.setTextColour(BLACK, WHITE);
        else
          screen.setTextColour(WHITE, BLACK);
        uint8_t note = midiDown[c];
        printNote(screen, note);
      }
    } else { // No notes held down, print last note noninverted
      screen.setTextColour(WHITE, BLACK);
      printNote(screen, lastMidi);
    }
  }
#endif

};

#endif   // __Midi2CVTriplet_hpp__
