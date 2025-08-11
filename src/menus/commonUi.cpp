#include "diva.h"

namespace commonUi {
using namespace diva;
i32 helpBaseId = 0;
bool out       = false;

HOOK (void, PlayControllerDisconnect, 0x140607BB0, void *a1, AetAction action) {
	AetLayerArgs helpBaseArgs ("AET_NSWGAM_CMN_MAIN", "cmn_win_help_base", 0x14, action);
	helpBaseArgs.play (&helpBaseId);
	if (action == AetAction::OUT_ONCE) out = true;
	originalPlayControllerDisconnect (a1, action);
}

bool
CommonUiDialogDisplay (u64 This) {
	auto args = (AetLayerArgs *)(This + 0x88);
	auto id   = args->unk_0x15C;

	AetComposition comp;
	GetComposition (&comp, id);

	auto btnArgs = (AetLayerArgs *)(This + 0x280);
	auto btnId   = btnArgs->unk_0x15C;

	if (auto btnLayer = aets->find (btnId)) {
		if (auto btnPlaceholder = comp.find ("savedata_warning_btn")) {
			btnLayer.value ()->color.w  = btnPlaceholder.value ()->opacity;
			btnLayer.value ()->position = btnPlaceholder.value ()->position;

			if (out && btnLayer.value ()->currentFrame >= btnLayer.value ()->endTime - 1) {
				StopAet (&btnArgs->unk_0x15C);
				btnArgs->unk_0x15C = 0;
				out                = false;
			}
		}
	}

	return false;
}

bool
CommonUiDialogDestroy (u64 This) {
	StopAet (&helpBaseId);
	return false;
}

void
init () {
	taskAddition addition;
	addition.display = CommonUiDialogDisplay;
	addition.destroy = CommonUiDialogDestroy;
	addTaskAddition ("CommonUIDialog", addition);

	INSTALL_HOOK (PlayControllerDisconnect);
	WRITE_NOP (0x140607FDF, 3);

	WRITE_MEMORY (0x140607A36, u32, 8); // Make text not force centered
}
} // namespace commonUi
