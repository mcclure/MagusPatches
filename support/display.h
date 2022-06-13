#ifndef __support_display_hpp__

#ifndef OWL_SIMULATOR

#ifndef BLACK
#error "MonochromeScreenPatch must be included before including this file"
// (It cannot be done here because MonochromeScreenPatch has no include guard)
#endif

#include "support/noteNames.h"

// Metrics: This stuff is not returned by the API so we have to hardcode it [to Magus specs]
#define CONSOLE_SIZE_X 21
#define CONSOLE_SIZE_Y 7
#define CONSOLE_STEP_Y 8
// If you want to draw a character at 0,0, you actually have to draw it at 0,8
#define CONSOLE_ZERO_Y (CONSOLE_STEP_Y)
#define CONSOLE_STEP_X 6

static char digitChar(uint8_t v) { // Get octave number of MIDI note (assume range 0-10)
	return '0' + v;
}

static char hexChar(uint8_t v) { // Get octave number of MIDI note (assume nothing)
	v %= 0x10;
	if (v < 10)
		return '0' + v;
	else
		return 'A' - 10 + v;
}

static char octaveChar(uint8_t note) { // Get octave number of MIDI note
	uint8_t octave = note / 12;
	if (octave == 0)
	  return '-'; // For -1
	return digitChar(octave);
}

static void printNote(MonochromeScreenBuffer& screen, uint8_t note) {
  screen.print(noteNames[note%12]);
  screen.write(octaveChar(note));
}

#endif

#endif // __support_display_hpp__