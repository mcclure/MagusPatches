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
  unsigned int seniority:2; // Enough for 3+1
  unsigned int lastMidi:8;
};

// Midi2CV but uses 3 bottom-area outlets. For the Chainsaw
class Midi2CVTripletPatch : public MidiPatchBase {
public:
  PackedHistory outHistory[MIDI_OUTS]; // Stack of notes down

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

  int highestSeniority() {
    int mostSenior = -1;
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate outs
      PackedHistory &out = outHistory[o];
      if ((int)out.seniority > mostSenior) {
        mostSenior = out.seniority;
      }
    }
    return mostSenior;
  }

  // Okay it turns out "natural feeling" note stealing logic is more complex than I thought
  void startNote(int at, uint8_t midiNote) {
    bool allPresent = true;

    // First see if there's a free note (depressed then released)
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate outs
      PackedHistory &out = outHistory[o];
      if (!out.present) {
        allPresent = false;              // Note free
      }
    }

    if (allPresent) { // Must steal note
      int mostSenior = highestSeniority();

      // Figure out which seniority level to steal from
      int targetSeniority = -1;
      int targetSeniorityCount = -1;
      for(int s = mostSenior; s >= 0; s--) { // Iterate seniorities
        int seniorityCount = 0;
        for(int o = 0; o < MIDI_OUTS; o++) { // Iterate outs
          PackedHistory &out = outHistory[o];
          if ((int)out.seniority == s) {
            seniorityCount++;                // Count outs with this seniority
          }
        }
        // Target a seniority for stealing if either it's the oldest seniority or it's using more than one note
        if (seniorityCount > 1 || targetSeniority < 0) {
          targetSeniority = s;
          targetSeniorityCount = seniorityCount;
          if (seniorityCount > 1) // Using more than one note is the strongest criterion
            break;
        }
      }

      bool pushSeniority = targetSeniorityCount > 1;

      // Replace oldest note
      for(int o = 0; o < MIDI_OUTS; o++) { // Iterate outs
        PackedHistory &out = outHistory[o];
        if ((int)out.seniority == targetSeniority) { // Found an output with the target seniority
          if (targetSeniorityCount > 1) { // If there's more than one output with this note, don't replace first
            out.seniority++;
            targetSeniorityCount = 0;     // Don't take this branch next output
          } else { // Code duplication :/
            out.present = true;
            out.seniority = 0;
            out.lastMidi = midiNote;
          }
        } else {
          out.seniority++;
        }
      }
    } else {
      for(int o = 0; o < MIDI_OUTS; o++) {
        PackedHistory &out = outHistory[o];
        if (out.present) { // Note is held, keep it but make it more senior
          out.seniority++;
        } else {            // Note is not held, replace it
          out.present = true;
          out.seniority = 0;
          out.lastMidi = midiNote;
          allPresent = false;
        }
      }
    }
  }

  void killNote(int at) {
    uint8_t killed = midiDown[at];
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
      PackedHistory &out = outHistory[o];
      if (out.present) {
        if (out.lastMidi == killed) {
          out.present = false;
        } else {
          out.seniority--; // At end of this loop lowest seniority should be 0
        }
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
  void processScreen(ScreenBuffer& screen){ // Print notes-playing array
    int uniqueNotes = highestSeniority()+1;
//debugMessage("Note count", downCount);
    int height = screen.getHeight();
    screen.clear();
    screen.setTextColour(WHITE, BLACK);
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values      
      PackedHistory &out = outHistory[o];

      screen.print((o*3+3)*8,16,""); // FIXME magic number?
      printNote(screen, out.lastMidi);
    }

    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
      screen.print((o*3+3)*8,32,""); // FIXME magic number?
      PackedHistory &out = outHistory[o];
      if (out.present) { // Highlight all currently keydowned notes by inverting
        screen.setTextColour(BLACK, WHITE);
        screen.print("   ");
      }
    }

    screen.setTextColour(WHITE, BLACK);
    for(int o = 0; o < MIDI_OUTS; o++) { // Iterate over possible values
      PackedHistory &out = outHistory[o];
      if (out.present) {
        screen.print((o*3+3)*8,48,""); // FIXME magic number?
        screen.write(digitChar(uniqueNotes - (int)out.seniority));
      }
    }
  }
#endif

};

#endif   // __Midi2CVTriplet_hpp__
