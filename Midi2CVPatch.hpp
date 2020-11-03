#ifndef __Midi2CV_hpp__
#define __Midi2CV_hpp__

// Midi input is converted to a CV/gate outputs. The notes currently down are displayed.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/
// If you reuse this code preserving credit is appreciated but not legally required.

#include "OpenWareMidiControl.h"
#include "support/noteNames.h"
#include "support/midi.h"
#include "VoltsPerOctave.h"
#include "basicmaths.h"

#define MAXDOWN 31
#define RETRIG_LENGTH 16

#ifdef OWL_SIMULATOR
#error "This requires MIDI and can't run in the simulator"
#endif

class Midi2CVPatch : public Patch {
private:
  uint8_t midiDown[MAXDOWN]; // Stack of notes down
  uint8_t downCount;         // Number of items in midiDown
  uint8_t lastMidi;          // Midi note currently being output
  bool isDown;               // Is GATE high?
  uint8_t needRetrig;           // Do we need to feather the gate next frame?
public:
  Midi2CVPatch(){        
    downCount = 0;
    lastMidi = MIDDLEC_MIDI;
    needRetrig = isDown = false;
  }

  ~Midi2CVPatch(){
  }

  char digitChar(uint8_t note) { // Get octave number of MIDI note
    uint8_t octave = note / 12;
    if (octave == 0)
      return '-'; // For -1
    return '0' - 1 + octave;
  }

  void processMidi(MidiMessage msg) { // Service MIDI note stack
      auto status = msg.getStatus();

      switch (status) {
        // Key on
        case MidiCodeNoteOn:
        case MidiCodeNoteOff: {
          auto midiNote = msg.getNote();

          // Check if this note is already somewhere in the midiDown stack.
          uint8_t matchAt, matchValue;
          bool match = false;
          for(int c = 0; c < downCount; c++) {
            matchValue = midiDown[c];
            if (matchValue == midiNote) {
              match = true;
              matchAt = c;
              break;
            }
          }

          // Matched. Either key lifted or there's a double down. Either way pull it from the stack
          if (match) {
            for(int c = matchAt; c<(int)downCount-1; c++)
              midiDown[c] = midiDown[c+1];
            downCount--;
          }

          switch (status) { // NoteOn and NoteOff implementations branch here
            case MidiCodeNoteOn: // On key down
              lastMidi = midiNote; // Set CV out
              if (downCount == MAXDOWN) { // We overflowed the stack. Forget the oldest note
                for(int c = 0; c < MAXDOWN-1; c++) {
                  midiDown[c] = midiDown[c+1];
                }
                downCount--;
              }
              midiDown[downCount] = midiNote; // Append to stack
              downCount++;
              if (isDown) // Retrig only if we were down before this
                needRetrig = RETRIG_LENGTH;
              isDown = true; // Set gate out
              break;
            case MidiCodeNoteOff: // On key up
              // Set retrig regardless; if isDown is set false it will be removed,
              // But if somehow we receive a down and up at once we'll want that retrig.
              if (matchValue == lastMidi) // Force retrigger if note we let go of was note playing
                needRetrig = RETRIG_LENGTH;
              if (downCount > 0) {
                lastMidi = midiDown[downCount-1]; // The new top of the stack becomes the new note.
              } else {
                isDown = false; // No more notes
              }
              break;
          }
        } break;
        default:break;
    }

  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples) {
  }

  void processAudio(AudioBuffer& buffer) { // Create CV/Gate from notes down
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    // Write samples
    float temp = isDown ? 1.0f : 0.0f;
    {
      int c = 0;
      for(; needRetrig && c < size; c++, needRetrig--) // First write needRetrig 0s
        leftData[c] = 0.0f;
      for(; c < size; c++) // Then if down write 1s
        leftData[c] = temp;
    }

    temp = (lastMidi - 33) / (12.0f * 5.0f); // We can output notes A1 to G#6
    for(int c = 0; c < size; c++)
      rightData[c] = temp;
}

#ifdef USE_SCREEN
  void printNote(ScreenBuffer& screen, uint8_t note) {
    screen.print(noteNames[note%12]);
    screen.write(digitChar(note));
  }
  void processScreen(ScreenBuffer& screen){ // Print notes-down stack
//debugMessage("Note count", downCount);
    bool first = true; // Print newest to oldest and invert the "current" note
    int height = screen.getHeight();
    screen.setTextColour(BLACK, WHITE);
    screen.clear();
    screen.print(0,8,""); // FIXME magic number?
    if (downCount > 0) {
      screen.setTextColour(BLACK, WHITE);
      for(int c = downCount-1; c >= 0; c--) {
        if (!first)
          screen.print(" ");
        uint8_t note = midiDown[c];
        printNote(screen, note);
        if (first) {
          screen.setTextColour(WHITE, BLACK);
          first = false;
        }
      }
    } else { // No notes held down, print last note noninverted
      screen.setTextColour(WHITE, BLACK);
      printNote(screen, lastMidi);
    }
  }
#endif

};

#endif   // __Midi2CV_hpp__
