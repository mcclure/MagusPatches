#ifndef __support_patchForSlot_hpp__
#define __support_patchForSlot_hpp__

#include "OpenWareMidiControl.h"

// Parameters are passed out (TOP ROW THEN BOTTOM ROW) horizontally
// But we want them to be passed out in 2x2 blocks of 4, left to right
PatchParameterId patchForSlot(int i) {
	return PatchParameterId((i & 1) | ( (i & 2) << 2 ) | ( (i & 12) >> 1 ));
}

#endif // __support_patchForSlot_hpp__
