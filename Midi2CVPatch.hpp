#ifndef __Midi2CV_hpp__
#define __Midi2CV_hpp__

// Does nothing. Prints numbers to the screen. To test drawing.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
#include "support/noteNames.h"
#include "support/midi.h"

#define MAXDOWN 31

#ifdef OWL_SIMULATOR
#error "This requires MIDI and can't run in the simulator"
#endif

class Midi2CVPatch : public Patch {
private:
  uint8_t midiDown[MAXDOWN]; // Stack of notes down
  uint8_t downCount;         // Number of items in midiDown
  uint8_t lastMidi;          // Midi note currently being output
  bool isDown;               // Is GATE high?
  bool needRetrig;           // Do we need to feather the gate next frame?
  //int debugLastStatus;
public:
  Midi2CVPatch(){        
    downCount = 0;
    lastMidi = MIDDLEC_MIDI;
    isDown = needRetrig = false;

    //debugLastStatus = 31337;
  }

  ~Midi2CVPatch(){
  }

  char digitChar(uint8_t note) {
    uint8_t octave = note / 12;
    if (note == 0)
      return '-';
    return '0' - 1 + octave;
  }

  void processMidi(MidiMessage msg){
      auto status = msg.getStatus();
//debugLastStatus = status;
      switch (status) {
        // Key on
        case MidiCodeNoteOn:
        case MidiCodeNoteOff: {
          auto midiNote = msg.getNote();

          // Check if this note is already somewhere in the midiDown stack.
          uint8_t matchAt;
          bool match = false;
          for(int c = 0; c < downCount; c++) {
            if (midiDown[c] == midiNote) {
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

          switch (status) {
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
                needRetrig = true;
              isDown = true; // Set gate out
              break;
            case MidiCodeNoteOff: // On key up
              // Set retrig regardless; if isDown is set false it will be removed,
              // But if somehow we receive a down and up at once we'll want that retrig.
              // HEY WAIT THIS IS WRONG. This will retrig when you keyup a misc note. FIXME
              needRetrig = true;
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

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void processAudio(AudioBuffer& buffer){
    FloatArray left = buffer.getSamples(LEFT_CHANNEL);
    FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

    // Buffers
    int size = min(left.getSize(), right.getSize());
    float *leftData = left.getData();
    float *rightData = right.getData();

    // Write sample
    memset(leftData,  0, size*sizeof(float));
    memset(rightData, 0, size*sizeof(float));
  }

#ifdef USE_SCREEN
  void printNote(ScreenBuffer& screen, uint8_t note) {
    screen.write(digitChar(note));
    screen.print(noteNames[note%12]);
  }
  void processScreen(ScreenBuffer& screen){
    debugMessage("Note count", downCount);
//debugMessage("last status", debugLastStatus);
    bool first = true; // Print newest to oldest and invert the "current" note
    screen.clear();
    screen.print(0,0,"");
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
    } else {
      screen.setTextColour(WHITE, BLACK);
      printNote(screen, lastMidi);
    }
  }
#endif

};

#endif   // __Saw4Patch_hpp__
