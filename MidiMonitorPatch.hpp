#ifndef __MidiMonitor_hpp__
#define __MidiMonitor_hpp__

// Print MIDI events to the screen.
// Currently will not work well with a continuous-streaming device (IE Midi Fighter)
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
#include "MonochromeScreenPatch.h"
#include "math.h"
#include "support/patchForSlot.h"
#include "support/midi.h"
#include "support/display.h"

#define MESSAGELINES 7

class MidiMonitorPatch : public MonochromeScreenPatch {
private:
  MidiMessage messageLines[MESSAGELINES];
  unsigned int messageLineCount;
  unsigned int rootId;

public:

  MidiMonitorPatch() {
    messageLineCount = 0;
    rootId = 0;
  }

  ~MidiMonitorPatch(){
  }

#ifndef OWL_SIMULATOR
  void processMidi(MidiMessage msg){
    if (messageLineCount > MESSAGELINES)
      messageLineCount = MESSAGELINES; // Should be impossible

    if (messageLineCount == MESSAGELINES) {
      for(int c = 0; c < MESSAGELINES-1; c++)
        messageLines[c] = messageLines[c+1];
    } else {
      messageLineCount++;
    }
    messageLines[messageLineCount-1] = msg;
    rootId = (rootId+1)%10;
  }

  void processScreen(MonochromeScreenBuffer& screen) {
    int cury = YZERO; // Magic number?

    screen.clear();
    
    if (messageLineCount == 0) {
      screen.setTextColour(WHITE, BLACK);
      screen.print(0,YZERO,"  Awaiting MIDI...");
    }

    for(int c = 0; c < messageLineCount; c++) {
      screen.setTextColour(WHITE, BLACK);
      screen.print(0,cury,"");

      screen.write(digitChar((rootId+c)%10));
      screen.write(' ');

      MidiMessage &msg = messageLines[c];
      screen.setTextColour(BLACK, WHITE);
      for(int d = 0; d < 4; d++) {
        screen.write(hexChar(msg.data[d] >> 8));
        screen.write(hexChar(msg.data[d]));
      }

      screen.setTextColour(WHITE, BLACK);
      screen.write(' ');

      screen.print("ch");
      screen.write(hexChar(msg.getChannel()));
      screen.write(' ');

      auto status = msg.getStatus();
      switch(status) {
        case NOTE_ON:
        case NOTE_OFF: {
          auto midiNote = msg.getNote();

          screen.print(status == NOTE_ON ? "ON  " : "OFF ");
          printNote(screen, midiNote);
        } break;
        case CONTROL_CHANGE: {
          screen.print("CC");
        } break;
        case PITCH_BEND_CHANGE: {
          screen.print("BEND");
        } break;
        default: {
          screen.write(hexChar(status >> 4));
          screen.write(hexChar(status));
        }
      }

      cury += YSTEP;
    }
  }
#endif

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  // Silence
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
};

#endif   // __Saw4Patch_hpp__
