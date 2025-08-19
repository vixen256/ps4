#include "diva.h"

namespace genericDialog {
using namespace diva;
i32 helpBaseId     = 0;
u64 *GenericDialog = (u64 *)0x14CC103A0;
bool playedOut     = false;
UpdateKeyAnmData *keyicon2Data;
UpdateKeyAnmData *keyicon3Data;
const char *keyStr = 0;

HOOK (string *, GetKeyStr, 0x1402B0F80, string *buf, u64 keyId) {
	if (keyStr) {
		buf->capacity = 15;
		buf->length   = strlen (keyStr);
		buf->ptr      = 0;
		strcpy (buf->data, keyStr);
		keyStr = 0;
		return buf;
	}
	return originalGetKeyStr (buf, keyId);
}

bool
GenericDialogDisplay (u64 This) {
	auto dialogAetLayerArgs = (AetLayerArgs *)(This + 0x78);
	auto pageNo             = *(u8 *)(This + 0x479);
	auto imgAetLayerArgs    = (AetLayerArgs *)(*(u64 *)(This + 0x498) + (0x1F8 * pageNo));
	auto compositionData    = (AetComposition *)(This + 0x468);
	auto dialogId           = *(i32 *)(This + 0x6C);

	compositionData->~AetComposition ();
	u8 buf[sizeof (AetComposition)];
	auto comp = new (buf) AetComposition;
	GetComposition (comp, dialogAetLayerArgs->id);
	*compositionData = *comp;

	if (dialogId == 0xC) return false;

	auto helpImgLayer = compositionData->find (string ("p_help_img_01_c"));
	if (!helpImgLayer) return false;
	if (auto aet = aets->find (imgAetLayerArgs->id)) {
		aet.value ()->position = helpImgLayer.value ()->position;
		aet.value ()->color.w  = helpImgLayer.value ()->opacity;
	}

	if (dialogId == 0x1E && pageNo == 1) {
		AetComposition imgAetLayerComp;
		GetComposition (&imgAetLayerComp, imgAetLayerArgs->id);
		if (auto layer = imgAetLayerComp.find (string ("helpwin_keyicon_02"))) {
			keyicon2Data->position = layer.value ()->position + helpImgLayer.value ()->position;
			keyicon2Data->opacity  = helpImgLayer.value ()->opacity;

			keyStr = "1";
			Vec2 a1;
			UpdateKeyAnm (&a1, keyicon2Data);
		}
		if (auto layer = imgAetLayerComp.find (string ("helpwin_keyicon_03"))) {
			keyicon3Data->position = layer.value ()->position + helpImgLayer.value ()->position;
			keyicon3Data->opacity  = helpImgLayer.value ()->opacity;

			keyStr = "3";
			Vec2 a1;
			UpdateKeyAnm (&a1, keyicon3Data);
		}
	}

	return false;
}

HOOK (void, GenericDialogPlay, 0x15ECE1320, void *a1, i32 dialogId) {
	if (dialogId == 0x0B || dialogId == 0x0C) return originalGenericDialogPlay (a1, dialogId);
	auto priority = 0x13;
	if (dialogId == 0x07) priority = 0x19;
	AetLayerArgs helpBaseArgs ("AET_NSWGAM_CMN_MAIN", "cmn_win_help_base", priority, AetAction::IN_LOOP);
	helpBaseArgs.play (&helpBaseId);
	playedOut = false;

	originalGenericDialogPlay (a1, dialogId);
}

HOOK (bool, DestroyGenericDialog, 0x1401B0480) {
	if (*GenericDialog == 0) return false;

	if (!playedOut) {
		playedOut  = true;
		auto layer = aets->find (helpBaseId);
		if (!layer) return originalDestroyGenericDialog ();

		AetLayerArgs helpBaseArgs ("AET_NSWGAM_CMN_MAIN", "cmn_win_help_base", 0x13, AetAction::OUT_ONCE);
		helpBaseArgs.play (&helpBaseId);
	}

	if (auto helpBase = aets->find (helpBaseId)) {
		if (helpBase.value ()->currentFrame >= helpBase.value ()->layer->endTime - 1) {
			StopAet (&helpBaseId);
			return originalDestroyGenericDialog ();
		}
		return true;
	} else {
		return originalDestroyGenericDialog ();
	}
}

HOOK (void, GenericDialogPlaySongSlect, 0x140664BD0, u64 This) {
	originalGenericDialogPlaySongSlect (This);
	if (IsSurvival ()) {
		auto comp             = (AetComposition *)(This + 0x400);
		*(u8 *)(This + 0x410) = 1; // Number of pages
		auto args             = *(AetLayerArgs **)(This + 0x430);
		StopAet (&args->id);
		StopAet (&((AetLayerArgs *)((u64)args + 0x1F8))->id);

		args->create ("AET_NSWGAM_CMN_MAIN", "helpwin_song_survival01", 0x19, AetAction::NONE);
		if (auto helpImgLayer = comp->find (string ("p_help_img_01_c"))) args->position = helpImgLayer.value ()->position;
		args->flags |= 8; // Hidden
		args->play (&args->id);
	}
}

extern "C" {
HOOK (void, SetKeyAnmOpacity, 0x14066705B);
f32
realSetKeyAnmOpacity (u64 This) {
	auto comp = (AetComposition *)(This + 0x400);
	if (auto layer = comp->find (string ("p_help_img_01_c"))) return layer.value ()->opacity;
	else return 1.0;
}
HOOK (void, PlayGenericDialogOut, 0x14066650B);
bool
realPlayGenericDialogOut (u64 This) {
	auto dialogId = *(i32 *)(This + 0x4);
	if (dialogId == 0x01) {
		auto args = (AetLayerArgs *)(This + 0x208);
		args->create ("AET_NSWGAM_CMN_MAIN", "cmn_msg_autosave2", 0, AetAction::OUT_ONCE);
		args->flags |= 8;
		args->play (&args->id);
		return false;
	} else if (dialogId == 0x0E || dialogId == 0x18 || dialogId == 0x21) {
		auto args = (AetLayerArgs *)(This + 0x208);
		args->create ("AET_NSWGAM_CMN_MAIN", "cmn_msg_autosave", 0, AetAction::OUT_ONCE);
		args->flags |= 8;
		args->play (&args->id);
		return false;
	} else if (dialogId == 0x02) {
		auto args = (AetLayerArgs *)(This + 0x208);
		args->create ("AET_NSWGAM_CMN_MAIN", "cmn_msg_difficulty", 0, AetAction::OUT_ONCE);
		args->flags |= 8;
		args->play (&args->id);
		return false;
	}
	return true;
}
HOOK (void, SetHelpPageNo10Color, 0x140667300);
HOOK (void, SetHelpPageNo01Color, 0x1406673AB);
}

void
init () {
	keyicon2Data = new UpdateKeyAnmData ();
	keyicon2Data->keycodes.push_back (Button::L2);
	keyicon2Data->xbSpriteId = 59958;
	keyicon2Data->psSpriteId = 59907;
	keyicon2Data->swSpriteId = 59945;
	keyicon2Data->stSpriteId = 59932;

	keyicon3Data = new UpdateKeyAnmData ();
	keyicon3Data->keycodes.push_back (Button::R2);
	keyicon3Data->xbSpriteId = 59960;
	keyicon3Data->psSpriteId = 59909;
	keyicon3Data->swSpriteId = 59947;
	keyicon3Data->stSpriteId = 59934;

	taskAddition addition;
	addition.display = GenericDialogDisplay;
	addTaskAddition ("GENERIC_DIALOG_SWITCH", addition);

	INSTALL_HOOK (GenericDialogPlay);
	INSTALL_HOOK (DestroyGenericDialog);
	INSTALL_HOOK (GenericDialogPlaySongSlect);

	WRITE_MEMORY (0x1401DEFB9, u8, 0x0F, 0xB6, 0xC0, 0x90, 0x90); // MOVZX EAX, AL
	WRITE_MEMORY (0x1406624AA, u8, 0x01);                         // Customization menus to 1 instead of 2
	WRITE_MEMORY (0x1406657FB, u8, 0x02);                         // helpwin_song_start menus to 2 instead of 3

	INSTALL_HOOK (GetKeyStr);
	INSTALL_HOOK (SetKeyAnmOpacity);
	INSTALL_HOOK (PlayGenericDialogOut);
	INSTALL_HOOK (SetHelpPageNo10Color);
	INSTALL_HOOK (SetHelpPageNo01Color);

	WRITE_MEMORY (0x14060AB29, u8, 0x8B, 0x4C, 0x24, 0x78, 0x89, 0x8D, 0xF0, 0x00, 0x00, 0x00);
	WRITE_NOP (0x14060ACE5, 9);

	WRITE_MEMORY (0x1406628D4, u8, 0x19);
	WRITE_MEMORY (0x1406629FF, u8, 0x1A);
	WRITE_MEMORY (0x1406672DE, i32, 0x1C);
	WRITE_MEMORY (0x140667389, i32, 0x1C);
	WRITE_MEMORY (0x140667065, i32, 0x1C);

	// sub_base_bg04::key_sub_base
	WRITE_MEMORY (0x14067AB53, u8, 0x8B, 0x50, 0x60, 0x89, 0x55, 0xC8, 0x90);
	WRITE_NOP (0x14067AB75, 3);
	WRITE_NOP (0x14067AB7B, 7);

	WRITE_MEMORY (0x140666BF5, u8, 0x11);
}
} // namespace genericDialog
