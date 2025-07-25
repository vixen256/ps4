#include "diva.h"

namespace pvWatch {
using namespace diva;
i32 footerButtonId      = 0;
u8 previousFooter       = 0;
u8 currentFooter        = 0;
u64 repeatPlaylistData  = 0;
u64 shufflePlaylistData = 0;
InputType previousInputType;

FUNCTION_PTR (bool, DecoIsActive, 0x1401FD7D0);

HOOK (void, ShowRepeatPlaylist, 0x140644B10, u64 a1, i32 action, bool a3) {
	repeatPlaylistData = a1;
	if (currentFooter != 3 && !DecoIsActive ()) return;
	originalShowRepeatPlaylist (a1, action, a3);
}

HOOK (void, ShowShufflePlaylist, 0x140644D40, u64 a1, i32 action, bool a3) {
	shufflePlaylistData = a1;
	if (currentFooter != 3 && !DecoIsActive ()) return;
	originalShowShufflePlaylist (a1, action, a3);
}

bool
PVWatchLoop (u64 a1) {
	if (DecoIsActive ()) {
		currentFooter  = 0;
		previousFooter = 0;
		StopAet (&footerButtonId);
	}
	InputType input = diva::getInputType ();
	if (input != previousInputType && currentFooter > 0) {
		previousInputType = input;
		char buf[128];
		sprintf (buf, "footer_button_%02d_%02d", currentFooter, (i32)input);
		diva::AetLayerArgs layer ("AET_NSWGAM_GAME_MAIN", buf, 0x1A, AetAction::LOOP);
		layer.play (&footerButtonId);
	}
	return false;
}

bool
PVWatchDestroy (u64 a1) {
	StopAet (&footerButtonId);
	repeatPlaylistData  = 0;
	shufflePlaylistData = 0;
	return false;
}

void
UpdateFooter (void *frameRateControl) {
	InputType input   = diva::getInputType ();
	previousInputType = input;
	char buf[128];
	if (currentFooter > 0) {
		sprintf (buf, "footer_button_%02d_%02d", currentFooter, (i32)input);
		diva::AetLayerArgs layer;
		if (currentFooter == 2) layer.create ("AET_NSWGAM_GAME_MAIN", buf, 0x1A, AetAction::LOOP);
		else layer.create ("AET_NSWGAM_GAME_MAIN", buf, 0x1A, AetAction::IN_LOOP);
		layer.frameRateControl = frameRateControl;
		layer.play (&footerButtonId);
	} else {
		sprintf (buf, "footer_button_%02d_%02d", previousFooter, (i32)input);
		diva::AetLayerArgs layer ("AET_NSWGAM_GAME_MAIN", buf, 0x1A, AetAction::OUT_ONCE);
		layer.frameRateControl = frameRateControl;
		layer.play (&footerButtonId);
	}
	previousFooter = currentFooter;
}

HOOK (void, UpdatePvWatchFooter, 0x140642750, u64 a1) {
	currentFooter = *(u8 *)(a1 + 0x2C) + 1;
	currentFooter %= 3;
	UpdateFooter ((void *)(a1 + 0x50 + 0x2C0));
	originalUpdatePvWatchFooter (a1);
}

HOOK (void, UpdatePlaylistWatchFooter, 0x1406440E0, u64 a1) {
	currentFooter = *(u8 *)(a1 + 0x2C) + 1;
	currentFooter %= 3;
	if (currentFooter == 1) currentFooter = 3;
	else if (currentFooter == 2) {
		originalShowRepeatPlaylist (repeatPlaylistData, 3, false);
		originalShowShufflePlaylist (shufflePlaylistData, 3, false);
	}
	UpdateFooter ((void *)(a1 + 0x90 + 0x2C0));
	originalUpdatePlaylistWatchFooter (a1);
}

void
init () {
	taskAddition addition;
	addition.loop    = PVWatchLoop;
	addition.destroy = PVWatchDestroy;
	addTaskAddition ("PV_WATCH", addition);
	INSTALL_HOOK (UpdatePvWatchFooter);
	INSTALL_HOOK (UpdatePlaylistWatchFooter);
	INSTALL_HOOK (ShowRepeatPlaylist);
	INSTALL_HOOK (ShowShufflePlaylist);

	// Stop hiding on pause
	WRITE_NOP (0x1406448A9, 5);
	// Move stickers prio from 0x0D to 0x03
	WRITE_MEMORY (0x1401FCAC3, u8, 0x03);
}
} // namespace pvWatch
