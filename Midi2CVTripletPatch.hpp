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
// If true, copy last value to audio out
#define AUDIO_OUT 1

struct PackedHistory {
  unsigned int present:1;
  unsigned int priority:2; // Enough for 3+1
  unsigned int lastMidi:8;
};

// Midi2CV but uses 3 bottom-area outlets. For the Chainsaw
class Midi2CVTripletPatch : public MidiPatchBase {
public:
  PackedHistory outHistory[AUDIO_OUT]; // Stack of notes down

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

      outHistory[c].present = false;
      outHistory[c].lastMidi = lastMidi;
    }
  }

  ~Midi2CVTripletPatch(){
  }

  void startNote(int at, uint8_t midiNote) {
    bool allPresent = true;

    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over output ports
      PackedHistory &out = outHistory[o];
      if (out.present) {
        out.priority++;
      } else {
        out.present = true;
        out.priority = 0;
        out.lastMidi = midiNote;
        allPresent = false;
      }
    }

    if (allPresent) { // Must kill note
      int highestPrio = 0;
      int highestPrioCount = 0;
      for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
        PackedHistory &out = outHistory[o];
        if (out.priority > highestPrio) {
          highestPrio = out.priority;
          highestPrioCount = 1;
        } else if (out.priority == highestPrio) {
          highestPrioCount++;
        }
      }

      for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
        PackedHistory &out = outHistory[o];
        if (out.priority == highestPrio) {
          if (highestPrioCount > 1) { // If highest priority is multiple keys, don't overwrite all
            highestPrioCount = 0;
          } else { // Code duplication :/
            out.present = true;
            out.priority = 0;
            out.lastMidi = midiNote;
          }
        }
      }
    }
  }

  void killNote(int at) {
    uint8_t killed = midiDown[at];
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
      PackedHistory &out = outHistory[o];
      if (out.present && out.lastMidi == killed) {
        out.present = false;
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

    int assign = lastMidi;
    int lastFoundAssign = -1;
    float lastValue;
    bool trig;

    PatchParameterId param = patchForSlot(PARAM_BASE);
    if (downCount) {
      if (needRetrig > 0) {
        needRetrig = needRetrig < size ? 0 : needRetrig - size;
        trig = false;
      } else {
        trig = true;
      }
    } else {
      trig = false;
    }

    setParameterValue(param, trig ? 1.0f : 0.0f);

    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
      PackedHistory &out = outHistory[o];
      assign = out.lastMidi;
      if (assign != lastFoundAssign) {
        lastFoundAssign = assign;
        lastValue = (assign - 33) / (12.0f * 5.0f); // We can output notes A1 to G#6
      }
      param = patchForSlot(PARAM_BASE + 1 + o);
      setParameterValue(param, lastValue /2.0f);
    }

#if AUDIO_OUT
    float trigValue = trig ? 1.0f : 0.0f;
    for(int c = 0; c < size; c++) {
      leftData[c] = trigValue;
      rightData[c] = lastValue;
    }
#else
    memset(leftData, 0, size*sizeof(float));
    memset(rightData, 0, size*sizeof(float));
#endif
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
        bool highlighted = false;
        uint8_t note = midiDown[c];
        for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
          PackedHistory &out = outHistory[o];
          if (out.present && out.lastMidi == note) {
            highlighted = true;
            break;
          }
        }
        if (highlighted) // Highlight all currently selected notes by inverting
          screen.setTextColour(BLACK, WHITE);
        else
          screen.setTextColour(WHITE, BLACK);
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
