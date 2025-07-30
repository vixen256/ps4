#include "diva.h"

namespace pause {
using diva::AetAction;
using diva::AetLayerArgs;

i32 pauseMenuBackgroundId = 0;
bool playedOut            = 0;

HOOK (void, LoadPauseBackground, 0x1406570E0, u64 a1, bool playOut) {
	if (playOut && !playedOut) {
		playedOut = true;
		AetLayerArgs pauseMenuBackground ("AET_NSWGAM_GAME_MAIN", "pause_win_add_base", 0x12, AetAction::OUT_ONCE);
		pauseMenuBackground.frameRateControl = (void *)(a1 + 0xB0);
		pauseMenuBackground.play (&pauseMenuBackgroundId);
	} else {
		playedOut = false;
		AetLayerArgs pauseMenuBackground ("AET_NSWGAM_GAME_MAIN", "pause_win_add_base", 0x12, AetAction::IN_LOOP);
		pauseMenuBackground.frameRateControl = (void *)(a1 + 0xB0);
		pauseMenuBackground.play (&pauseMenuBackgroundId);
	}
	originalLoadPauseBackground (a1, playOut);
}

HOOK (i32 *, GetPauseTouched, 0x140659100, u64 a1, u64 a2, u64 a3) {
	i32 *val = originalGetPauseTouched (a1, a2, a3);
	if (diva::IsSurvival () && val[0] == 1) {
		val[0] = 22;
		val[1] = 0;
	}
	return val;
}

HOOK (void, PauseExit, 0x14065B810, u64 a1) {
	if (!playedOut) {
		playedOut = true;
		AetLayerArgs pauseMenuBackground ("AET_NSWGAM_GAME_MAIN", "pause_win_add_base", 0x12, AetAction::OUT_ONCE);
		pauseMenuBackground.frameRateControl = (void *)(a1 + 0xB0);
		pauseMenuBackground.play (&pauseMenuBackgroundId);
	}
	originalPauseExit (a1);
}

bool
PauseDestroy (u64 task) {
	diva::StopAet (&pauseMenuBackgroundId);
	return false;
}

void
init () {
	INSTALL_HOOK (LoadPauseBackground);
	INSTALL_HOOK (GetPauseTouched);
	INSTALL_HOOK (PauseExit);

	diva::taskAddition addition;
	addition.destroy = PauseDestroy;
	diva::addTaskAddition ("PAUSE_MENU_SWITCH", addition);
}
} // namespace pause
