#ifndef __midiPatchBase_hpp__
#define __midiPatchBase_hpp__

// Midi input is tracked for whatever purpose. The notes currently down are displayed.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/
// If you reuse this code preserving credit is appreciated but not legally required.

#include "OpenWareMidiControl.h"
#include "support/noteNames.h"
#include "support/midi.h"
#include "basicmaths.h"

#define MIDI_MAXDOWN 31
#define MIDI_RETRIG_LENGTH 16

class MidiPatchBase : public Patch {
protected:
  uint8_t midiDown[MIDI_MAXDOWN]; // Stack of notes down
  uint8_t downCount;         // Number of items in midiDown
  uint8_t lastMidi;          // Midi note currently being output
  bool isDown;               // Is GATE high?
  uint8_t needRetrig;           // Do we need to feather the gate next frame?
public:
  MidiPatchBase(){        
    downCount = 0;
    lastMidi = MIDDLEC_MIDI;
    needRetrig = isDown = false;
  }

  ~MidiPatchBase(){
  }

  char digitChar(uint8_t note) { // Get octave number of MIDI note
    uint8_t octave = note / 12;
    if (octave == 0)
      return '-'; // For -1
    return '0' - 1 + octave;
  }

  virtual void startNote(int at, uint8_t midiNote) {
  }

  virtual void killNote(int at) {
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
            killNote(matchAt);
            for(int c = matchAt; c<(int)downCount-1; c++)
              midiDown[c] = midiDown[c+1];
            downCount--;
          }

          switch (status) { // NoteOn and NoteOff implementations branch here
            case MidiCodeNoteOn: // On key down
              lastMidi = midiNote; // Set CV out
              if (downCount == MIDI_MAXDOWN) { // We overflowed the stack. Forget the oldest note
                for(int c = 0; c < MIDI_MAXDOWN-1; c++) {
                  midiDown[c] = midiDown[c+1];
                }
                downCount--;
              }
              midiDown[downCount] = midiNote; // Append to stack
              downCount++;
              startNote(downCount-1, midiNote);
              if (isDown) // Retrig only if we were down before this
                needRetrig = MIDI_RETRIG_LENGTH;
              isDown = true; // Set gate out
              break;
            case MidiCodeNoteOff: // On key up
              // Set retrig regardless; if isDown is set false it will be removed,
              // But if somehow we receive a down and up at once we'll want that retrig.
              if (matchValue == lastMidi) // Force retrigger if note we let go of was note playing
                needRetrig = MIDI_RETRIG_LENGTH;
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

#ifdef USE_SCREEN
  void printNote(ScreenBuffer& screen, uint8_t note) {
    screen.print(noteNames[note%12]);
    screen.write(digitChar(note));
  }
  void processScreen(ScreenBuffer& screen){ // Print notes-down stack
//debugMessage("Note count", downCount);
    bool first = true;
    int height = screen.getHeight();
    screen.setTextColour(BLACK, WHITE);
    screen.clear();
    screen.print(0,8,""); // FIXME magic number?
    if (downCount > 0) {
      screen.setTextColour(WHITE, BLACK);
      for(int c = 0; c < downCount; c++) {
        if (!first) { // Print space between values
          screen.print(" ");
        } else {
          first = false;
        }
        if (c == downCount-1) // Highlight the currently selected note by inverting it
          screen.setTextColour(BLACK, WHITE);
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

#endif   // __midiPatchBase_hpp__
