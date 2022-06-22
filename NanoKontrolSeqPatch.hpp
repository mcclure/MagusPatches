#ifndef __NanoKontrolSeq_hpp__
#define __NanoKontrolSeq_hpp__

// Works with a Korg nanoKONTROL2 (assumes port 0 channel 0)
// Acts like a sequencer for the parameter ports (assumes Magus)
// Author Andi McClure. Released under MIT license https://choosealicense.com/licenses/mit/

#include "OpenWareMidiControl.h"
#include "MonochromeScreenPatch.h"
#include "support/display.h"

// Constants

#define LANE_COUNT 8
#define NOTE_COUNT (LANE_COUNT)
#define SONG_COUNT (LANE_COUNT)
// This lane is the "BPM"
#define LANE_PERIOD 7
#define LANE_LTICK 0
#define LANE_RTICK 1
#define LANE_PERIODMODE_SPECIAL(x) ((x)==LANE_PERIOD||(x)==LANE_LTICK||(x)==LANE_RTICK)
#define KNOB_MIDPOINT 64
// The knob crosses two slider ticks on either side. Set to 1.0 for finest control
#define KNOB_MAG 2.0
#define KNOB_RADIX (127.0/KNOB_MAG)
// Includes all lightable uniques, including PLAY
#define UNIQUE_LIT_COUNT 6
// // If 0, will run for only 24 hours and 51 minutes and then there will be an audible glitch.
// // If 1, will be able to run continuously for 12 million years
// #define LONG_RUNNING 0 // Wait... do I need a global timer?
#define DEFAULT_BPM 140
#define TICK_SUSTAIN 8

// If you see "unvirtual" on a method, it does nothing, I'm just documenting that
// this method CANNOT be made virtual because it is called from the constructor
// API methods assumed constructor-safe: getBlockSize, getSampleRate
#define unvirtual

// Categories for CC database
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

// IDs for CC_GROUP_UNIQUE_LIT/CC_GROUP_UNIQUE_UNLIT
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

  CC_UNIQUE_COUNT,
  CC_UNIQUE_LITROOT = CC_UNIQUE_SHIFT
};

// Item in the "CC database", which has one item for each button/control
struct CcInfo {
  uint8_t cc;      // MIDI CC value
  uint8_t group;   // CcGroup
  uint8_t id;      // CcUniqueId
  int8_t lightIdx; // Index in lightOn/lightDbIdx, always -1 for non-light buttons
};

// This requires a special NanoKontrol2 configuration where the CCs are
// carefully chosen to prevent collisions with the default Magus control CCs
#define CC_COUNT 51
#define LIGHT_COUNT 30
CcInfo ccDb[CC_COUNT] = {  // Comment indicates NanoKontrol2 default CC
  {2, CC_GROUP_SLIDER, 0}, // 0
  {3, CC_GROUP_SLIDER, 1}, // 1
  {4, CC_GROUP_SLIDER, 2}, // 2
  {5, CC_GROUP_SLIDER, 3}, // 3
  {6, CC_GROUP_SLIDER, 4}, // 4
  {7, CC_GROUP_SLIDER, 5}, // 5
  {8, CC_GROUP_SLIDER, 6}, // 6
  {9, CC_GROUP_SLIDER, 7}, // 7

  {10, CC_GROUP_KNOB, 0},  // 16
  {11, CC_GROUP_KNOB, 1},  // 17
  {14, CC_GROUP_KNOB, 2},  // 18
  {15, CC_GROUP_KNOB, 3},  // 19
  {16, CC_GROUP_KNOB, 4},  // 20
  {17, CC_GROUP_KNOB, 5},  // 21
  {18, CC_GROUP_KNOB, 6},  // 22
  {19, CC_GROUP_KNOB, 7},  // 23

  {25, CC_GROUP_SOLO, 0},  // 32
  {26, CC_GROUP_SOLO, 1},  // 33
  {27, CC_GROUP_SOLO, 2},  // 34
  {28, CC_GROUP_SOLO, 3},  // 35
  {29, CC_GROUP_SOLO, 4},  // 36
  {30, CC_GROUP_SOLO, 5},  // 37
  {31, CC_GROUP_SOLO, 6},  // 38
  {32, CC_GROUP_SOLO, 7},  // 39

  {33, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_PLAY}, // 41
  {34, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_STOP},   // 42
  {35, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_REW},    // 43
  {36, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_FF},     // 44
  {37, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_REC},    // 45
  {38, CC_GROUP_UNIQUE_LIT, CC_UNIQUE_SHIFT},  // 46

  {39, CC_GROUP_MUTE, 0},  // 48
  {40, CC_GROUP_MUTE, 1},  // 49
  {41, CC_GROUP_MUTE, 2},  // 50
  {42, CC_GROUP_MUTE, 3},  // 51
  {43, CC_GROUP_MUTE, 4},  // 52
  {44, CC_GROUP_MUTE, 5},  // 53
  {45, CC_GROUP_MUTE, 6},  // 54
  {46, CC_GROUP_MUTE, 7},  // 55

  {47, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_SONG_L}, // 58
  {48, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_SONG_R}, // 59

  {49, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_CLICK}, // 60
  {50, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_L},     // 61
  {51, CC_GROUP_UNIQUE_UNLIT, CC_UNIQUE_MENU_R},     // 62

  {52, CC_GROUP_RECORD, 0}, // 64
  {53, CC_GROUP_RECORD, 1}, // 65
  {54, CC_GROUP_RECORD, 2}, // 66
  {55, CC_GROUP_RECORD, 3}, // 67
  {56, CC_GROUP_RECORD, 4}, // 68
  {57, CC_GROUP_RECORD, 5}, // 69
  {58, CC_GROUP_RECORD, 6}, // 70
  {59, CC_GROUP_RECORD, 7}, // 71
};

#if 0
#if LONG_RUNNING
typedef uint64_t TimeCode;
#else
typedef uint32_t TimeCode;
#endif
#endif

// Remember lanes make up a note, notes make up a song

struct Note {
  uint8_t slider[LANE_COUNT]; // 0-127
  uint8_t knob[LANE_COUNT];   // 0-127
};

struct Song {
  Note notes[NOTE_COUNT];
  uint32_t period;            // Step length in samples
  int8_t tick[2];            // bitshift for L and R channel click tracks (0x80 == 0)
  uint8_t notesLive;          // How many steps before repeat
};

enum SongState {
  SongDrone, // Stopped, but all triggers are open
  SongStop,  // Stopped, and no triggers are open
  SongPlay,  // Playing
};
#if 0
enum ButtonState {
  ButtonNeutral = 0,
  ButtonPressed = 1,
  ButtonToggle  = 2,
  ButtonBlink   = 3,

  ButtonBlinkShift = 2
};
#endif

#define BZERO(field) memset(field, 0, sizeof(field))

class NanoKontrolSeqPatch : public MonochromeScreenPatch {
private:
    // Display state
    bool lightOn[LIGHT_COUNT], lightOnWas[LIGHT_COUNT]; // lightIdx to ON (current frame, last frame)
    uint8_t lightDbIdx[LIGHT_COUNT]; // lightIdx to DB idx
    uint8_t dbRootRec, dbRootMute, dbRootSolo, dbUniqueLit[UNIQUE_LIT_COUNT]; // db idx
    bool uniqueLitDown[UNIQUE_LIT_COUNT]; // id minus CC_UNIQUE_LITROOT
    bool writeDown[LANE_COUNT];
    bool lockDown, performDown; // Are we banned from using sliders? Are we in the special "send only" mode?
    //uint8_t buttonState[CC_UNIQUE_COUNT]; // ButtonState // TODO clarify difference from uniqueLitDown

    // UI state // TODO: "Apps"?
    bool songPeriodMode;
    Note songPeriodNote; // FIXME: Store one lane not all 8!
    Note lastValue;

    // Song
    Song song;
    int32_t songId;

    // Player state
    SongState playing;
    uint8_t noteAt;  // Current sequencer step
//    TimeCode timeAt; // Current time progression
    int32_t nextStep;  // How many samples to next sample?
    int32_t stepCount; // How many steps to next tick? (WIP)

    bool needLights; // If true, lightOn has been updated

    int debug1, debug2; // DELETE ME

public:
  NanoKontrolSeqPatch(){
    BZERO(lightOn);
    BZERO(lightOnWas);
    // BZERO(dbUniqueLit); // If DB is messed up and does not contain all 6 unique lits, this will prevent crash
    songPeriodMode = false;
    noteAt = 0;
//    timeAt = 0;
    nextStep = 0;
    needLights = true;
    lockDown = false;
    performDown = false;
    loadResource(0);
    memcpy(&lastValue, &song.notes[0], sizeof(lastValue));

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
#define SPECIALLY_IDENTIFY_PLAY(info) ((info).group == CC_GROUP_UNIQUE_UNLIT && (info).id == CC_UNIQUE_PLAY)
      if (info.group == CC_GROUP_MUTE || info.group == CC_GROUP_SOLO || info.group == CC_GROUP_RECORD
       || info.group == CC_GROUP_UNIQUE_LIT || SPECIALLY_IDENTIFY_PLAY(info)) {
        info.lightIdx = lightIdx;
        lightDbIdx[lightIdx] = c;

        // Mark location of important lights
        if (info.id == 0) {
          if (info.group == CC_GROUP_RECORD)
            dbRootRec = c;
          else if (info.group == CC_GROUP_MUTE)
            dbRootMute = c;
          else if (info.group == CC_GROUP_SOLO)
            dbRootSolo = c;
        } else if (info.group == CC_GROUP_UNIQUE_LIT || SPECIALLY_IDENTIFY_PLAY(info)) {
          dbUniqueLit[info.id-CC_UNIQUE_LITROOT] = c;
        }

        lightIdx++;
      } else {
        info.lightIdx = -1;
      }
      lightSet(info.cc, false);
    }
  }

  ~NanoKontrolSeqPatch(){
  }

  // Reset a song to defaults
  unvirtual void initSong(Song &target) {
    for(int c = 0; c < NOTE_COUNT; c++) {
      BZERO(target.notes[c].slider);
      memset(target.notes[c].knob, KNOB_MIDPOINT, sizeof(target.notes[c].knob));
    }
    target.period = defaultPeriod();
    memset(target.tick, 1, sizeof(target.tick));
    target.notesLive = NOTE_COUNT;
  }

  // Switch to a specific resource
  unvirtual void loadResource(int _songId) {
    songId = _songId;
    char name[12] = "nk2seqX.dat";
    name[6] = songId+1;
    Resource* resource = NULL; //getResource(name);
    if (resource && resource->getSize() >= (sizeof(Song) + 4)) {
      uint8_t *data = (uint8_t *)resource->getData();
      //uint32_t *idWord = data;
      data += 4;
      memcpy(&song, data, sizeof(song));
    } else {
      initSong(song);
    }
  }

  void saveResource() {
  #if 0
    char name[12] = "nk2seqX.dat";
    name[6] = songId+1;
    size_t size = sizeof(Song)+4;
    uint8_t *data = (uint8_t *)malloc(size);
    uint32_t *idWord = (uint32_t *)data;
    *idWord = 0x1;
    data++;
    memcpy(data, sizeof(Song), &song);
    Resource *resource = new Resource(name, size, data);
    Resource::destroy(resource);
    free(data);
  #endif
  }

  // Hard set a light on or off (bypasses caching)
  void lightSet(unsigned int cc, bool on) {
    MidiMessage msg((CONTROL_CHANGE) >> 4, CONTROL_CHANGE, cc, on?127:0);
    sendMidi(msg);
  }

  // Call when something is about to happen that could change light state
  void readyLights() {
    if (!needLights) { // Don't stomp existing lights state 
      memcpy(lightOnWas, lightOn, sizeof(lightOnWas));
      BZERO(lightOn);
      needLights = true; // FIXME: Could be more efficient by setting this selectively when state changes
    }
  }

  // Call when you are done potentially-updating lights
  void updateLights() {
    if (needLights) {
      // Set "obvious" state
      if (songPeriodMode) { // All bets off
        lightOn[ccDb[dbRootMute].lightIdx + LANE_LTICK] = true;
        lightOn[ccDb[dbRootMute].lightIdx + LANE_RTICK] = true;
        lightOn[ccDb[dbRootMute].lightIdx + LANE_PERIOD] = true;
      }
      { // Set exactly one R
        const int8_t rootLightIdx = ccDb[dbRootRec].lightIdx;
        if (songPeriodMode || performDown || !shiftDown()) { // Regular: Stepping
          lightOn[rootLightIdx + noteAt] = true;
        } else {
          for(int c = 0; c < song.notesLive; c++)
            lightOn[rootLightIdx + c] = true;
        }
      }
      { // Set all relevant S'es
        const int8_t rootLightIdx = ccDb[dbRootSolo].lightIdx;
        for(int c = 0; c < LANE_COUNT; c++)
          if (writeDown[c])
            lightOn[rootLightIdx + c] = true;
      }

      // Set play if playing
      uniqueLitDown[CC_UNIQUE_PLAY-CC_UNIQUE_LITROOT] = playing == SongPlay;
      uniqueLitDown[CC_UNIQUE_STOP-CC_UNIQUE_LITROOT] = playing == SongStop;

      // Send light changes
      for(unsigned int c = 0; c < UNIQUE_LIT_COUNT; c++)
        lightOn[ccDb[dbUniqueLit[c]].lightIdx] = uniqueLitDown[c];
      if (lockDown)
        lightOn[ccDb[dbUniqueLit[CC_UNIQUE_REW-CC_UNIQUE_LITROOT]].lightIdx] = true;
      if (performDown)
        lightOn[ccDb[dbUniqueLit[CC_UNIQUE_FF-CC_UNIQUE_LITROOT]].lightIdx] = true;
      for(unsigned int c = 0; c < LIGHT_COUNT; c++) {
        unsigned int cc = ccDb[lightDbIdx[c]].cc;
        if (lightOn[c] != lightOnWas[c]) {
          lightSet(cc, lightOn[c]);
        }
      }
      needLights = false;
    }
  }

  // Set a parameter from lane values
  void paramSet(uint8_t lane, Note &n) {
    setParameterValue((PatchParameterId)(0+lane), 
      n.slider[lane]/127.0f+
      (n.knob[lane]-KNOB_MIDPOINT)/(KNOB_MIDPOINT*KNOB_RADIX));
  }

  void noteStep() {
    if (noteAt >= song.notesLive-1)
      noteAt = 0;
    else
      noteAt++;
  }

  void noteChanged() {
    Note &n = song.notes[noteAt];
    for(int c = 0; c < LANE_COUNT; c++)
      paramSet(c, n);
  }

  // Convert DEFAULT_BPM to a sample period
  unvirtual float defaultPeriod() {
    return getSampleRate()*60.0f/DEFAULT_BPM;
  }

  // Round a period to a "usable" BPM
  unvirtual uint32_t roundPeriod(float period) {
    uint32_t blockSize = getBlockSize();
    float div = roundf(period/blockSize)*blockSize;
    return div<blockSize ? blockSize : (uint32_t)div;
  }

  bool shiftDown() {
    return uniqueLitDown[CC_UNIQUE_SHIFT-CC_UNIQUE_LITROOT]; 
  }

  // Process MIDI
  void processMidi(MidiMessage msg){
debug1 = -2; debug2 = -2;
    // Ignore non-CC MIDI
    if ((msg.data[1] & 0xF0) == CONTROL_CHANGE) {
      const uint8_t &cc = msg.data[2];
      const uint8_t &value = msg.data[3];

      // Binary search for ccIdx (see note on ccDB)
      uint8_t ccIdx = CC_COUNT/2;
debug1 = -1; debug2 = 0;
      {
        uint8_t ccIdxLow = 0;
        uint8_t ccIdxHigh = CC_COUNT-1;
        while (1) {
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
          if (ccIdxLow >= ccIdxHigh) {
            debug1 = -cc;
            return; // Unrecognized CC, cancel everything
          }
          ccIdx = newCcIdx;
        }
      }

      // Have a known CC, now act
      debug1 = ccDb[ccIdx].cc;
      debug2 = -ccIdx;

      // Lights may change after this point
      readyLights();

      // Handle control
      CcInfo &info = ccDb[ccIdx];
      bool performTriggered = false;
      bool laneValueChange = false;
#define LANE_ALLOWED(lane) (!lockDown || writeDown[lane])
#define LANE_WRITE(lane, field, value) { \
        if (!(songPeriodMode && !LANE_PERIODMODE_SPECIAL(lane))) { \
          laneValueChange = true; \
          if (performDown) { \
            performTriggered = true; \
          } else { \
            (songPeriodMode ? \
              songPeriodNote : \
              song.notes[noteAt] \
            ).field[lane] = value; \
          } \
        } \
      }
#define LANE_WRITE_SOLO(lane) { \
        LANE_WRITE(lane, slider, lastValue.slider[lane]); \
        LANE_WRITE(lane, knob, lastValue.knob[lane]); \
      }

      switch (info.group) {
        case CC_GROUP_SLIDER: {
          lastValue.slider[info.id] = value;
          if (LANE_ALLOWED(info.id)) {
            LANE_WRITE(info.id, slider, value);
          }
        } break;
        case CC_GROUP_KNOB: {
          lastValue.knob[info.id] = value;
          if (LANE_ALLOWED(info.id)) {
            LANE_WRITE(info.id, knob, value);
          }
        } break;
        case CC_GROUP_RECORD: {
          if (!shiftDown()) { // Normal: Change lane
            noteAt = info.id;
            noteChanged();
          } else {            // Shifted: Change song-loop length
            song.notesLive = info.id + 1;
          }
        } break;
        case CC_GROUP_SOLO: {
          bool down = value>0;
          writeDown[info.id] = down;

          if (down) {
            LANE_WRITE_SOLO(info.id);
          }
        } break;
        case CC_GROUP_UNIQUE_LIT:
          uniqueLitDown[info.id-CC_UNIQUE_LITROOT] = value>0;
          // FALL THROUGH:
        case CC_GROUP_UNIQUE_UNLIT: {
          if (value>0) {
            switch (info.id) {
              case CC_UNIQUE_PLAY: { // PLAY BUTTON
                if (playing == SongPlay) {
                  playing = SongStop;
                } else {
                  playing = SongPlay;
                }
              } break;
              case CC_UNIQUE_STOP: { // STOP BUTTON
                if (shiftDown() && lockDown) {
                  performDown = !performDown;
                } else {
                  playing = SongStop; // This is OK because drone does nothing yet!
                  // playing = shiftDown() || playing == SongDrone ? SongStop : SongDrone;
                  noteAt = 0;
                  stepCount = 0;
                }
              } break;
              case CC_UNIQUE_FF: {   // FAST-FORWARD
                if (!shiftDown()) {  // Regular
                  noteStep();
                  noteChanged();
                } else {             // Timeshift
                  songPeriodMode = true;
                  // Must write songPeriodNote based on current period
                  uint32_t songPeriod = song.period;
                  // THE FOLLOWING IS WRONG
                  songPeriod /= getBlockSize();
                  songPeriodNote.knob[LANE_PERIOD] = songPeriod % 128;
                  songPeriod /= 128;
                  songPeriodNote.slider[LANE_PERIOD] = songPeriod > 127 ? 127 : songPeriod;
                }
              } break;
              case CC_UNIQUE_REW: {  // REWIND
                if (!shiftDown()) {
                  if (noteAt == 0)
                    noteAt = song.notesLive-1;
                  else
                    noteAt--;
                  noteChanged();
                } else {
                  lockDown = !lockDown;
                  if (!lockDown)
                    performDown = false;
                }
              } break;
              case CC_UNIQUE_REC: {
                if (!shiftDown()) { // Save
                  saveResource();
                } else { // Load
                  loadResource(songId);
                }
              } break;
#if 0
              case CC_UNIQUE_SONG_L: {
                loadResource((songId - 1 + SONG_COUNT)%SONG_COUNT);
              } break;
              case CC_UNIQUE_SONG_R: {
                loadResource((songId + 1)%SONG_COUNT);
              } break;
#endif
            }
          }
        } break;
        default:break;
      }

      // Control handled, execute consequences
      // A lane changed
      if (laneValueChange) {
        if (songPeriodMode) {
          switch (info.id) {
            case LANE_LTICK:
            case LANE_RTICK: {
              uint8_t tickId = info.id-LANE_LTICK;
              int8_t &tick = song.tick[tickId];
              tick = value; tick -= 62; tick /= 4; // -62 so bottom two ticks exactly are -16/+16
              if (tick == -1 || tick == 1)
                tick = 1;
              debug1 = tickId;
              debug2 = tick;
            } break;
            case LANE_PERIOD: {
              uint32_t songPeriod = song.period;
              // Equation from eyeballing it: (x*4-1)^3 + 5*x + 1 from x=0 to x=1 (doesn't work)
              // Try 1/60 instead (bad)
              float x = songPeriodNote.slider[LANE_PERIOD]/128.0 + (songPeriodNote.knob[LANE_PERIOD]+1)*KNOB_MAG/128.0/128.0;
              debug1 = x * 128 * 128;
              x = 240.0f / x;
              song.period = roundPeriod(x);
              nextStep += (song.period - songPeriod);
              debug2 = song.period;
            } break;
          }
        } else { // Regular
          debug1 = info.id + 1;
          debug2 = value;
          paramSet(info.id, performTriggered ? lastValue : song.notes[noteAt]);
        }
      }

      // Handle modes/apps
      if (songPeriodMode && !(shiftDown() && uniqueLitDown[CC_UNIQUE_FF-CC_UNIQUE_LITROOT]))
        songPeriodMode = false;

      // Lights may have changed
      updateLights();
    }
  }

  void buttonChanged(PatchButtonId bid, uint16_t value, uint16_t samples){
  }

  // Process audio 
  void processAudio(AudioBuffer& buffer){
    float *leftData, *rightData;
    int bufferSize;
    { // Audio silence
      FloatArray left = buffer.getSamples(LEFT_CHANNEL);
      FloatArray right = buffer.getSamples(RIGHT_CHANNEL);

      // Buffers
      bufferSize = min(left.getSize(), right.getSize());
      leftData = left.getData();
      rightData = right.getData();

      // Write sample
      memset(leftData,  0, bufferSize*sizeof(float));
      memset(rightData, 0, bufferSize*sizeof(float));
    }

    uint32_t timeStep = getBlockSize();
    if (playing == SongPlay) {
      // Tick refers to "click track" code
      // This is WIP and pretty bad
      // The correct(?) way to do this is probably to make a 16th note one block
      for(int ch = 0; ch < 2; ch++) {
        int tickEvery = song.period;
        int tickOffset = nextStep >= 0 ? song.period - nextStep : 0; // "progress" 
        int8_t &tick = song.tick[ch];
        if (tick < 0) {
          tickEvery /= (-tick);
          tickOffset %= tickEvery;
        } else if (tick > 0) {
          tickEvery *= tick;
          tickOffset += song.period*(stepCount % tick);
        }
        for(int c = tickOffset; c < bufferSize; c += tickEvery) {
          for(int d = 0; d < TICK_SUSTAIN && (c+d)<bufferSize; d++) {
            (ch ? rightData : leftData)[c+d] = 1;
          }
        }
      }

      nextStep -= timeStep;

      if (nextStep <= 0) {
        readyLights();

        noteStep();
        stepCount++;
        nextStep = song.period;

        if (!performDown) {
          for (int c = 0; c < LANE_COUNT; c++) {
            bool laneValueChange, performTriggered; // Unused
            if (writeDown[c]) { // FIXME: This writes tempo in song period mode, but that's unnecessary
              LANE_WRITE_SOLO(c);
            }
          }

          noteChanged();
        } else { // For performDown do clumsy filtered version of noteChanged
          for(int c = 0; c < LANE_COUNT; c++)
            paramSet(c, writeDown[c] ? lastValue : song.notes[noteAt]);
        }
      }
    }
    updateLights(); // Catch any straggling light changes
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

  // Redraw screen
  // TODO: Only do this when screenChanged
  void processScreen(MonochromeScreenBuffer& screen) {
    screen.clear();

    const int cury = CONSOLE_ZERO_Y+CONSOLE_STEP_Y;
    const int curx = (CONSOLE_SIZE_X-2)*CONSOLE_STEP_X;
    printLeft(screen, curx, cury, debug1);
    printLeft(screen, curx, cury+CONSOLE_STEP_Y, debug2);
  }
};

#endif   // __NanoKontrolSeq_hpp__
