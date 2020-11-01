#ifndef __ScreenSaver_hpp__
#define __ScreenSaver_hpp__

// Does nothing. Prints numbers to the screen. To test drawing.
// Author Andi McClure. License https://creativecommons.org/publicdomain/zero/1.0/

#include "OpenWareMidiControl.h"

#define ACCUMULATOR_START 0x9A9A9A9A
#define ACCUMULATOR_MULT  0xA9A9A9A9
#define ACCUMULATOR_ADD   0x87654321
#define ACCUMULATOR_SHIFT 8

#define BUFFER_SIZE 33

class ScreenSaverPatch : public Patch {
private:
  uint32_t frame;
  uint32_t accumulator; // RNG
  char tempBuffer[BUFFER_SIZE];
public:
  ScreenSaverPatch(){        
    frame = 1;
    accumulator = ACCUMULATOR_START;
    nextRandom(); nextRandom();
  }

  ~ScreenSaverPatch(){
  }

  void processMidi(MidiMessage msg){
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  uint32_t nextRandom() {
    accumulator *= ACCUMULATOR_MULT;
    accumulator += ACCUMULATOR_ADD;
    accumulator >>= ACCUMULATOR_SHIFT;
    return accumulator;
  }

  char *a2c(uint32_t n) {
    char *at = &tempBuffer[BUFFER_SIZE-1];
    *at = '\0';
    do  {
      *(--at) = '0' + (n % 10);
      n /= 10;
    } while (n);
    return at;
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
  void processScreen(ScreenBuffer& screen){
    int height = screen.getHeight();
    int width = screen.getWidth();
    uint32_t entropy = nextRandom();

    bool invert = entropy & 1;
    entropy >>= 1;
    uint32_t y = entropy % height;
    entropy /= height;
    uint32_t x = entropy % width;

    const char *num = a2c(frame++);

    if (invert)
      screen.setTextColour(BLACK, WHITE);
    else
      screen.setTextColour(WHITE, BLACK);

    screen.clear();
    screen.print(x, y, num);
  }
#else
  #error "This patch doesn't really work without a screen"
#endif

};

#endif   // __Saw4Patch_hpp__
