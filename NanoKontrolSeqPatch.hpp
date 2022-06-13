#ifndef __NanoKontrolSeq_hpp__
#define __NanoKontrolSeq_hpp__

// Works with a Korg nanoKONTROL2 (assumes port 0 channel 0)
// Acts like a sequencer for the parameter ports (assumes Magus)
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"
#include "MonochromeScreenPatch.h"
#include "support/display.h"

enum CcGroup {
  CC_GROUP_SLIDER,
  CC_GROUP_KNOB,
  CC_GROUP_RECORD,
  CC_GROUP_MUTE,
  CC_GROUP_SOLO,
  CC_GROUP_TRANSPORT,
  CC_GROUP_UNIQUE_LIT, // Means autolit-- "play" is an unlit bc lights are manual
  CC_GROUP_UNIQUE_UNLIT,
};

// Also counts as knob/SMR count
#define SLIDER_COUNT 8
#define SLIDER_META_ALLOW true

enum CcUniqueId {
  CC_UNIQUE_SONG_L,
  CC_UNIQUE_SONG_R,
  CC_UNIQUE_MENU_CLICK,
  CC_UNIQUE_MENU_L,
  CC_UNIQUE_MENU_R,
  CC_UNIQUE_SHIFT,
  CC_UNIQUE_REW,
  CC_UNIQUE_FF,
  CC_UNIQUE_STOP,
  CC_UNIQUE_PLAY,
  CC_UNIQUE_REC,
};

struct CcInfo {
  uint8_t cc;
  uint8_t group;
  uint8_t id;
  int8_t lightIdx; // Value undefined for non-light buttons
};

// This could be much simpler if I switched out the supported nanoKontrol2 profile
// to one where there were no gaps in the CC order. However I want to stay as close
// to the default spec as possible to not interfere with other apps using the device
#define CC_COUNT 51
#define LIGHT_COUNT 30
CcInfo ccDb[CC_COUNT] = {
  {0, CC_GROUP_SLIDER, 0},
  {1, CC_GROUP_SLIDER, 1},
  {2, CC_GROUP_SLIDER, 2},
  {3, CC_GROUP_SLIDER, 3},
  {4, CC_GROUP_SLIDER, 4},
  {5, CC_GROUP_SLIDER, 5},
  {6, CC_GROUP_SLIDER, 6},
  {7, CC_GROUP_SLIDER, 7},

  {16, CC_GROUP_KNOB, 0},
  {17, CC_GROUP_KNOB, 1},
  {18, CC_GROUP_KNOB, 2},
  {19, CC_GROUP_KNOB, 3},
  {20, CC_GROUP_KNOB, 4},
  {21, CC_GROUP_KNOB, 5},
  {22, CC_GROUP_KNOB, 6},
  {23, CC_GROUP_KNOB, 7},

  {32, CC_GROUP_SOLO, 0},
  {33, CC_GROUP_SOLO, 1},
  {34, CC_GROUP_SOLO, 2},
  {35, CC_GROUP_SOLO, 3},
  {36, CC_GROUP_SOLO, 4},
  {37, CC_GROUP_SOLO, 5},
  {38, CC_GROUP_SOLO, 6},
  {39, CC_GROUP_SOLO, 7},

  {41, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_PLAY},
  {42, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_STOP},
  {43, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_REW},
  {44, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_FF},
  {45, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_REC},
  {46, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_SHIFT},

  {48, CC_GROUP_MUTE, 0},
  {49, CC_GROUP_MUTE, 1},
  {50, CC_GROUP_MUTE, 2},
  {51, CC_GROUP_MUTE, 3},
  {52, CC_GROUP_MUTE, 4},
  {53, CC_GROUP_MUTE, 5},
  {54, CC_GROUP_MUTE, 6},
  {55, CC_GROUP_MUTE, 7},

  {58, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_SONG_L},
  {59, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_SONG_R},

  {60, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_CLICK},
  {61, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_L},
  {62, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_R},

  {64, CC_GROUP_RECORD, 0},
  {65, CC_GROUP_RECORD, 1},
  {66, CC_GROUP_RECORD, 2},
  {67, CC_GROUP_RECORD, 3},
  {68, CC_GROUP_RECORD, 4},
  {69, CC_GROUP_RECORD, 5},
  {70, CC_GROUP_RECORD, 6},
  {71, CC_GROUP_RECORD, 7},
};

#define BZERO(field) memset(field, 0, sizeof(field))

class NanoKontrolSeqPatch : public MonochromeScreenPatch {
private:
    bool lightOn[LIGHT_COUNT];
    uint8_t lightCcIdx[LIGHT_COUNT];

    int debug1, debug2; // DELETE ME

public:
  NanoKontrolSeqPatch(){
    BZERO(lightOn);

    // Register all ports as outputs
    char scratch[5] = {0,0, 0, '>',0};
    for(int c = 0; c < 16; c++) {
      int id = c;
      if (c < 8) {
        scratch[0] = 'C'; // FIXME: Meta lane will be incorrect here 
        scratch[1] = 'V';
      } else {
        scratch[0] = 'T';
        scratch[1] = 'G';
        id /= 2;
      }
      scratch[2] = '1' + id;
      registerParameter((PatchParameterId)c, scratch);
    }

    // Build light DB and turn off all lights as we go
    int lightIdx = 0;
    for(unsigned int c = 0; c < CC_COUNT; c++) {
      CcInfo &info = ccDb[c];
      if (info.group == CC_GROUP_MUTE || info.group == CC_GROUP_SOLO || info.group == CC_GROUP_RECORD
       || info.group == CC_GROUP_UNIQUE_LIT || (info.group == CC_GROUP_UNIQUE_UNLIT && info.id == CC_UNIQUE_PLAY)) {
        info.lightIdx = lightIdx;
        lightCcIdx[lightIdx] = c;
        lightIdx++;
      } else {
        info.lightIdx = -1;
      }
      lightSet(info.cc, false);
    }
  }

  ~NanoKontrolSeqPatch(){
  }

  void lightSet(unsigned int cc, bool on) {
    MidiMessage msg((CONTROL_CHANGE) >> 4, CONTROL_CHANGE, cc, on?127:0);
    sendMidi(msg);
  }

  void processMidi(MidiMessage msg){
debug1 = -2; debug2 = -2;
    if ((msg.data[1] & 0xF0) == CONTROL_CHANGE) {
      const uint8_t &cc = msg.data[2];
      const uint8_t &value = msg.data[3];
      uint8_t ccIdx = CC_COUNT/2;
debug1 = -1; debug2 = 0;
      { // Binary search for ccIdx (see note on ccDB)
        uint8_t ccIdxLow = 0;
        uint8_t ccIdxHigh = CC_COUNT-1;
        while (1) {
if (debug2 > 10) { debug1 = cc; return; }
          CcInfo &info = ccDb[ccIdx];
          int newCcIdx;
          debug2++;
          if (cc == info.cc) {
            break;
          } else if (cc < info.cc) { // New upper bound
            ccIdxHigh = ccIdx;
            newCcIdx = ccIdxLow + (ccIdx-ccIdxLow)/2;
            if (ccIdx == newCcIdx)
              newCcIdx--;
          } else { // New lower bound
            ccIdxLow = ccIdx;
            newCcIdx = ccIdx + (ccIdxHigh-ccIdx)/2;
            if (ccIdx == newCcIdx)
              newCcIdx++;
          }
          if (ccIdxLow >= ccIdxHigh)
            return; // Unrecognized CC!
          ccIdx = newCcIdx;
        }
      }

      // Now act
      debug1 = ccDb[ccIdx].cc;
    }
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  void processAudio(AudioBuffer& buffer){
//    float timeStep = ((float)getBlockSize()/getSampleRate());

    { // Audio silence
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
  }

  // Print an integer, right-aligned
  void printLeft(MonochromeScreenBuffer& screen, int x, int y, int num) {
    bool neg = num<0;
    if (neg) num = -num;
    do {
      screen.print(x, y, "");
      screen.write('0' + (uint8_t)(num%10));
      num /= 10;
      x -= CONSOLE_STEP_X;
    } while (num>0);
    if (neg) {
      screen.print(x, y, "");
      screen.write('-');
    }
  }

  void processScreen(MonochromeScreenBuffer& screen) {
    screen.clear();

    const int cury = CONSOLE_ZERO_Y+CONSOLE_STEP_Y;
    const int curx = (CONSOLE_SIZE_X-2)*CONSOLE_STEP_X;
    printLeft(screen, curx, cury, debug1);
    printLeft(screen, curx, cury+CONSOLE_STEP_Y, debug2);
  }
};

#endif   // __NanoKontrolSeq_hpp__
