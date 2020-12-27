#!/usr/bin/env python
# Ingest source for a Rebel Technology Magus patch, emit an executable that runs it and emits float data
# Tested on Python 2.7.16
# Unless otherwise noted, author is Andi McClure, license https://creativecommons.org/publicdomain/zero/1.0/
# Some sections from OpenWare/OwlProgram headers. I do not believe these to be copyrightable.

import sys
import re
import shutil
import os
import os.path
import tempfile
import subprocess
try:
    import click
except ImportError:
    sys.stderr.write("Error: \"Click\" module missing. Run `pip install click`\n")
    sys.exit(1)

# Convert "/path/to/file/filename.ext" to "filename"
chopdot = re.compile(r'^[^\.]+')
def innerName(s):
    s = os.path.basename(s)
    chopped = chopdot.search(s)
    return chopped and chopped.group(0) or s

# Convert dest:inpath to [dest, inpath] or [None, inpath]
splitColon = re.compile(r'^([^\:]*)\:(.+)')
def includeSplit(path):
  match = splitColon.match(path)
  if match:
    d = match.group(1)
    f = match.group(2)
  else:
    d = None
    f = path
  return [d, os.path.abspath(f)]

# Given [dest, inpath] and a build dir, create dir path and return dest, inpath
def prepUnpackPair(pair, buildDir):
  d, filename = pair
  if d:
    d = os.path.join(buildDir, d)
    if not os.path.exists(d):
      os.makedirs(d)
  else:
    d = buildDir
  return d, filename

@click.command(help="Processes an hpp file into a standalone executable simulating running on the Magus")
@click.argument('infile')
@click.option('--class', '-c', '_class', type=click.STRING, help="Name of main class (if different from infile name)")
@click.option('--output', '-o', type=click.STRING, help="Output file        (if different from infile name)")
@click.option('--include', '-i', multiple=True, type=click.STRING, help="Copy this file into build directory (Note: If a destination directory is needed, prefix with :\nEG --include \"support:support/file.h\"")
@click.option('--note', '-n', multiple=True, type=click.STRING, help="Play MIDI note into program. Syntax 69 for note 69 on at start, 100:69 or 100:69:1 for note 69 on at sample 100, or 200:69:0 for note 69 off at sample 200.")
@click.option('--cxx', envvar='CXX', default="c++", type=click.STRING, help="(Or env var CXX) C++ compiler to use")
def make(infile, _class, cxx, output, include, note):
    # Clean up arguments, make all paths absolute except infile
    defaultName = innerName(infile)
    if not defaultName:
        raise click.ClickException("INFILE appears to be a directory?")
    if not _class:
        _class = defaultName
    if not output:
        output = os.path.abspath(defaultName)
    include = [includeSplit(x) for x in ([infile] + list(include))]
    infile = os.path.basename(infile)
    sampleRate = 44100
    # print(infile, _class, cxx, output, include) # Debug

    # Copy files into temp dir
    # TODO: Some method for copying files to subdirs
    buildDir = tempfile.mkdtemp()
    print("Building in directory "+buildDir)
    os.chdir(buildDir)
    for pair in include:
      d, filename = prepUnpackPair(pair, buildDir)
      shutil.copy(filename, d)

    notes = []
    if note:
      for _n in note:
        n = _n.split(":")
        if len(n) == 1:
          at,n,on = 0,int(_n),True
        elif len(n) == 2:
          at,n,on = int(n[0]),int(n[1]),True
        elif len(n) == 3:
          at,n,on = int(n[0]),int(n[1]),int(n[2])
        else:
          raise click.ClickException("Don't understand "+_n)
        notes.append([at, [on and 9 or 8, on and 0x90 or 0x80, n & 0x7F, on and 0x7F or 0]])

    # Create include file
    with open("__SIM_INCLUDE.h", "w") as f:
        f.write("""
#ifndef ____SIM_INCLUDE
#define ____SIM_INCLUDE

// Miniature, wildly incorrect implementation of openware/OwlProgram classes
// TODO: Pull in more code from actual openware/OwlProgram repos?

#import <algorithm>
#import <vector>
using std::min;
using std::max;

#define OWL_SIMULATOR 1

// Enum copied from Openware repo, git:76c941b2e7b2, OpenWareMidiControl.h
enum PatchParameterId {{
  PARAMETER_A,
  PARAMETER_B,
  PARAMETER_C,
  PARAMETER_D,
  PARAMETER_E,
  PARAMETER_F,
  PARAMETER_G,
  PARAMETER_H,

  PARAMETER_AA,
  PARAMETER_AB,
  PARAMETER_AC,
  PARAMETER_AD,
  PARAMETER_AE,
  PARAMETER_AF,
  PARAMETER_AG,
  PARAMETER_AH,

  PARAMETER_BA,
  PARAMETER_BB,
  PARAMETER_BC,
  PARAMETER_BD,
  PARAMETER_BE,
  PARAMETER_BF,
  PARAMETER_BG,
  PARAMETER_BH,

  PARAMETER_CA,
  PARAMETER_CB,
  PARAMETER_CC,
  PARAMETER_CD,
  PARAMETER_CE,
  PARAMETER_CF,
  PARAMETER_CG,
  PARAMETER_CH,

  PARAMETER_DA,
  PARAMETER_DB,
  PARAMETER_DC,
  PARAMETER_DD,
  PARAMETER_DE,
  PARAMETER_DF,
  PARAMETER_DG,
  PARAMETER_DH,
}};

// Enum copied from Openware repo, git:76c941b2e7b2, OpenWareMidiControl.h
enum PatchButtonId {{
  BYPASS_BUTTON,
  PUSHBUTTON,
  GREEN_BUTTON,
  RED_BUTTON,
  BUTTON_A,
  BUTTON_B,
  BUTTON_C,
  BUTTON_D,
  BUTTON_E,
  BUTTON_F,
  BUTTON_G,
  BUTTON_H,
  GATE_BUTTON = 0x7f,
  MIDI_NOTE_BUTTON = 0x80 // "values over 127 are mapped to note numbers"
}};

struct Patch {{
    std::vector<float> _parameters;
    void registerParameter(PatchParameterId _id, const char *) {{
        int need = (int)_id + 1;
        if (_parameters.size() <= need) _parameters.resize(need);
    }}
    float getParameterValue(PatchParameterId id) {{ return _parameters[(int)id]; }} // TODO
    void  setParameterValue(PatchParameterId id, float v) {{ _parameters[(int)id] = v; }}     // TODO
    float getSampleRate() {{ return {sampleRate}; }}
}};

struct FloatArray {{
    size_t _size;
    float *_data;
    void _clear() {{
        memset(_data, 0, _size);
    }}

    float *getData() {{ return _data; }}
    size_t getSize() {{ return _size; }}
}};

#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

struct AudioBuffer {{
    FloatArray _left, _right;
    void _clear() {{
        _left._clear();
        _right._clear();
    }}
    AudioBuffer(size_t capacity) {{
        _left._data = (float *)malloc(capacity*sizeof(float));
        _left._size = capacity;
        _right._data = (float *)malloc(capacity*sizeof(float));
        _right._size = capacity;
    }}
    ~AudioBuffer() {{
        free(_left._data);
        free(_right._data);
    }}

    FloatArray getSamples(int idx) {{
        return idx == LEFT_CHANNEL ? _left : _right;
    }}
}};

// Copied from OwlProgram repo, git:2918c483c53c, MidiStatus.h

enum MidiStatus {{
  STATUS_BYTE     = 0x80,
  NOTE_OFF      = 0x80,
  NOTE_ON     = 0x90,
  POLY_KEY_PRESSURE   = 0xA0,
  CONTROL_CHANGE    = 0xB0,
  PROGRAM_CHANGE    = 0xC0,
  CHANNEL_PRESSURE    = 0xD0,
  PITCH_BEND_CHANGE   = 0xE0,
  SYSTEM_COMMON     = 0xF0,
  SYSEX       = 0xF0,
  TIME_CODE_QUARTER_FRAME       = 0xF1,
  SONG_POSITION_PTR             = 0xF2,
  SONG_SELECT                   = 0xF3,
  RESERVED_F4                   = 0xF4,
  RESERVED_F5                   = 0xF5,
  TUNE_REQUEST                  = 0xF6,
  SYSEX_EOX                     = 0xF7,   
  SYSTEM_REAL_TIME    = 0xF8,
  TIMING_CLOCK            = 0xF8,
  RESERVED_F9                   = 0xF9,
  START                         = 0xFA,
  CONTINUE                      = 0xFB,
  STOP                          = 0xFC,
  RESERVED_FD                   = 0xFD,
  ACTIVE_SENSING                = 0xFE,
  SYSTEM_RESET                  = 0xFF,
  MIDI_CHANNEL_MASK   = 0x0F,
  MIDI_STATUS_MASK    = 0xF0
}};

enum MidiControlChange {{
  MIDI_CC_MODULATION    = 0x01,
  MIDI_CC_BREATH        = 0x02,
  MIDI_CC_VOLUME        = 0x07,
  MIDI_CC_BALANCE       = 0x08,
  MIDI_CC_PAN           = 0x0a,
  MIDI_CC_EXPRESSION    = 0x0b,
  MIDI_CC_EFFECT_CTRL_1 = 0x0c,
  MIDI_CC_EFFECT_CTRL_2 = 0x0d,
  MIDI_ALL_SOUND_OFF    = 0x78,
  MIDI_RESET_ALL_CTRLS  = 0x79,
  MIDI_LOCAL_CONTROL    = 0x7a,
  MIDI_ALL_NOTES_OFF    = 0x7b,
  MIDI_OMNI_MODE_OFF    = 0x7c,
  MIDI_OMNI_MODE_ON     = 0x7d,
  MIDI_MONO_MODE_ON     = 0x7e,
  MIDI_POLY_MODE_ON     = 0x7f
}};

enum UsbMidi {{
  USB_COMMAND_MISC                = 0x00, /* reserved */
  USB_COMMAND_CABLE_EVENT         = 0x01, /* reserved */
  USB_COMMAND_2BYTE_SYSTEM_COMMON = 0x02, /* e.g. MTC, SongSelect */
  USB_COMMAND_3BYTE_SYSTEM_COMMON = 0x03, /* e.g. Song Position Pointer SPP */
  USB_COMMAND_SYSEX               = 0x04,
  USB_COMMAND_SYSEX_EOX1          = 0x05,
  USB_COMMAND_SYSEX_EOX2          = 0x06,
  USB_COMMAND_SYSEX_EOX3          = 0x07,
  USB_COMMAND_NOTE_OFF            = 0x08,
  USB_COMMAND_NOTE_ON             = 0x09,
  USB_COMMAND_POLY_KEY_PRESSURE   = 0x0A,
  USB_COMMAND_CONTROL_CHANGE    = 0x0B,
  USB_COMMAND_PROGRAM_CHANGE    = 0x0C,
  USB_COMMAND_CHANNEL_PRESSURE    = 0x0D,
  USB_COMMAND_PITCH_BEND_CHANGE   = 0x0E,
  USB_COMMAND_SINGLE_BYTE   = 0x0F
}};

enum OwlProtocol {{
  OWL_COMMAND_DISCOVER            = 0xa0,
  OWL_COMMAND_PARAMETER           = 0xb0,
  OWL_COMMAND_COMMAND             = 0xc0,
  OWL_COMMAND_MESSAGE             = 0xd0,
  OWL_COMMAND_DATA                = 0xe0,
  OWL_COMMAND_RESET               = 0xf0,
}};

// Copied from OwlProgram repo, git:2918c483c53c, MidiMessage.h

class MidiMessage {{
 public:
  union {{
    uint32_t packed;
    uint8_t data[4];
  }};
  MidiMessage():packed(0){{}}
  MidiMessage(uint32_t msg): packed(msg){{}}
  MidiMessage(uint8_t port, uint8_t d0, uint8_t d1, uint8_t d2){{
    data[0] = port;
    data[1] = d0;
    data[2] = d1;
    data[3] = d2;
  }}
  uint8_t getPort(){{
    return (data[0] & 0xf0)>>4;
  }}
  uint8_t getChannel(){{
    return (data[1] & MIDI_CHANNEL_MASK);
  }}
  uint8_t getStatus(){{
    return (data[1] & MIDI_STATUS_MASK);
  }}
  uint8_t getNote(){{
    return data[2];
  }}
  uint8_t getVelocity(){{
    return data[3];
  }}
  uint8_t getControllerNumber(){{
    return data[2];
  }}
  uint8_t getControllerValue(){{
    return data[3];
  }}
  uint8_t getChannelPressure(){{
    return data[2];
  }}
  uint8_t getProgramChange(){{
    return data[1];
  }}
  int16_t getPitchBend(){{
    int16_t pb = (data[2] | (data[3]<<7)) - 8192;
    return pb;
  }}
  bool isNoteOn(){{
    return ((data[1] & MIDI_STATUS_MASK) == NOTE_ON) && getVelocity() != 0;
  }}
  bool isNoteOff(){{
    return ((data[1] & MIDI_STATUS_MASK) == NOTE_OFF) || (((data[1] & MIDI_STATUS_MASK) == NOTE_ON) && getVelocity() == 0);
  }}
  bool isControlChange(){{
    return (data[1] & MIDI_STATUS_MASK) == CONTROL_CHANGE;
  }}
  bool isProgramChange(){{
    return (data[1] & MIDI_STATUS_MASK) == PROGRAM_CHANGE;
  }}
  bool isChannelPressure(){{
    return (data[1] & MIDI_STATUS_MASK) == CHANNEL_PRESSURE;
  }}
  bool isPitchBend(){{
    return (data[1] & MIDI_STATUS_MASK) == PITCH_BEND_CHANGE;
  }}
  static MidiMessage cc(uint8_t ch, uint8_t cc, uint8_t value){{
    return MidiMessage(USB_COMMAND_CONTROL_CHANGE, CONTROL_CHANGE|(ch&0xf), cc&0x7f, value&0x7f);
  }}
  static MidiMessage pc(uint8_t ch, uint8_t pc){{
    return MidiMessage(USB_COMMAND_PROGRAM_CHANGE, PROGRAM_CHANGE|(ch&0xf), pc&0x7f, 0);
  }}
  static MidiMessage pb(uint8_t ch, int16_t bend){{
    bend += 8192;
    return MidiMessage(USB_COMMAND_PITCH_BEND_CHANGE, PITCH_BEND_CHANGE|(ch&0xf), bend&0x7f, (bend>>7)&0x7f);
  }}
  static MidiMessage note(uint8_t ch, uint8_t note, uint8_t velocity){{
    if(velocity == 0)
      return MidiMessage(USB_COMMAND_NOTE_OFF, NOTE_OFF|(ch&0xf), note&0x7f, velocity&0x7f);
    else
      return MidiMessage(USB_COMMAND_NOTE_ON, NOTE_ON|(ch&0xf), note&0x7f, velocity&0x7f);
  }}
  static MidiMessage cp(uint8_t ch, uint8_t value){{
    return MidiMessage(USB_COMMAND_CHANNEL_PRESSURE, CHANNEL_PRESSURE|(ch&0xf), value&0x7f, 0);
  }}
}};

#endif

""".format(sampleRate=sampleRate))

    # Create include file forwards
    # TODO: Make a fuller list, make a command line arg
    forwardIncludes = ["OpenWareMidiControl.h", "StompBox.h"]
    for name in forwardIncludes:
        with open(name, "w") as f:
            f.write("""
#include "__SIM_INCLUDE.h"
""")

    # Create driver file
    # TODO: Take input values for knobs
    # TODO: Take sample rate value
    # TODO: Emit a wav header?
    with open("__driver.cpp", "w") as f:
        f.write("""
#include <string>
#include <vector>
#include <algorithm>

const char *explanation =
    "Generates a number of samples from {_class} ({infile}) and prints them to stdout as interleaved float samples. To open, try import raw data feature in Amadeus or Audacity.";

const char *usage =
    "Usage: %s [OPTIONS]\\n\\n"
    "-s, --samples: Number of samples (default {sampleRate})\\n"
    "-h, --human: Print human readable instead of machine samples\\n"
    "-help, --help: Print this message\\n";

void bailError(const std::string &name, const std::string &err) {{
    fprintf(stderr, "Error: %s\\n\\n", err.c_str());
    fprintf(stderr, usage, name.c_str());
    exit(1);
}}

#include "{infile}"

#define NOTECOUNT {noteLen}

int noteAt[NOTECOUNT] = {{{noteAt}}};
MidiMessage notes[NOTECOUNT] = {{{noteContent}}};
int processingNote = 0;

int main(int argc, char **argv) {{
    int samples = {sampleRate};
    bool human = false;
    const int frameSize = 1024;

    for (int c = 1; c < argc; c++) {{
        std::string arg = argv[c];
        if (arg == "--help" || arg == "-help") {{
            printf("%s\\n\\n", explanation);
            printf(usage, argv[0]);
            exit(0); // BAIL OUT
        }} else if (arg == "-s" || arg == "--samples") {{
            if (c+1 >= argc)
                bailError(argv[0], arg + " missing parameter");
            samples = atoi(argv[c+1]);
            c++;
        }} else if (arg == "-h" || arg == "--human") {{
            human = true;
        }}
    }}

    {_class} generator;
    AudioBuffer buffer(frameSize);
    std::vector<float> mix;

    for(int off = 0; off < samples; off += frameSize) {{
        int currentFrameSize = std::min(frameSize, samples-off);
        buffer._left._size  = currentFrameSize;
        buffer._right._size = currentFrameSize;
        buffer._clear();

        if (processingNote < NOTECOUNT && noteAt[processingNote] <= off) {{
          generator.processMidi(notes[processingNote]);
          processingNote++;
        }}

        generator.processAudio(buffer);

        if (human) {{
            for(int idx = 0; idx < currentFrameSize; idx++)
                printf("%8.8f %8.8f\\n", buffer._left._data[idx], buffer._right._data[idx]);
        }} else {{
            mix.reserve(currentFrameSize*2);
            mix.clear();
            for(int idx = 0; idx < currentFrameSize; idx++) {{
                mix.push_back(buffer._left._data[idx]);
                mix.push_back(buffer._right._data[idx]);
            }}
            fwrite(&mix[0], sizeof(float), currentFrameSize*2, stdout);
        }}
    }}

    return 0;
}}
""".format(infile=infile, _class=_class, sampleRate=sampleRate, noteLen=len(notes),
  noteAt=", ".join([str(n[0]) for n in notes]),
  noteContent=", ".join(
      [
        (
          "MidiMessage(" +
          ", ".join(
            [hex(b) for b in n[1]]
          )
          + ")"
        ) for n in notes
      ]
    )
  ))

    # Compile
    result = subprocess.call([cxx, "__driver.cpp", "-I.", "-o", output])

    sys.exit(result)

make()
