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

@click.command(help="Processes an hpp file into a standalone executable simulating running on the Magus")
@click.argument('infile')
@click.option('--class', '-c', '_class', type=click.STRING, help="Name of main class")
@click.option('--output', '-o', type=click.STRING, help="Output file")
@click.option('--include', '-i', multiple=True, type=click.STRING, help="Copy this file into build directory")
@click.option('--cxx', envvar='CXX', default="c++", type=click.STRING, help="(Or env var CXX) C++ compiler to use")
def make(infile, _class, cxx, output, include):
    # Clean up arguments, make all paths absolute except infile
    defaultName = innerName(infile)
    if not defaultName:
        raise click.ClickException("INFILE appears to be a directory?")
    if not _class:
        _class = defaultName
    if not output:
        output = os.path.abspath(defaultName)
    include = [os.path.abspath(x) for x in ([infile] + list(include))]
    infile = os.path.basename(infile)
    sampleRate = 44100
    # print(infile, _class, cxx, output, include) # Debug

    # Copy files into temp dir
    # TODO: Some method for copying files to subdirs
    buildDir = tempfile.mkdtemp()
    print("Building in directory "+buildDir)
    os.chdir(buildDir)
    for filename in include:
        shutil.copy(filename, buildDir)

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
""".format(infile=infile, _class=_class, sampleRate=sampleRate))

    # Compile
    result = subprocess.call([cxx, "__driver.cpp", "-o", output])

    sys.exit(result)

make()
