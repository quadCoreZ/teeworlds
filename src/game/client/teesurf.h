#ifndef __TEESURF_H_
#define __TEESURF_H_

#include <base/vmath.h>

enum {

	TS_SCORE_TEAMCOLORS=1,
	TS_SCORE_TEAMSIZE=2,
	TS_SCORE_FRIENDS=4,
	TS_SCORE_KILLS=8,
	TS_SCORE_DEATHS=16,

	TS_HUD=1,
	TS_HUD_SCORE=2,
	TS_HUD_SCORE_FLAGRESPAWN=4,
	TS_HUD_SCORE_FLASH=8,
	TS_HUD_HPAMMO_VANILLA=16,
	TS_HUD_SMOKE=32,
};

#endif