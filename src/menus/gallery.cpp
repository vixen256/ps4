#include "diva.h"

using namespace diva;
extern i32 theme;

namespace gallery {
i32 menuTxt1Id    = 0;
i32 menuTxt2Id    = 0;
i32 menuTxt3Id    = 0;
i32 menuTxt4Id    = 0;
i32 menuTxt5Id    = 0;
i32 menuTxtBaseId = 0;

void
playGalleryTxt (i32 button, AetAction action) {
	AetLayerArgs menuTxtData;
	switch (button) {
	case 0:
		menuTxtData.create ("AET_PS4_GALLERY_MAIN", "menu_txt_01", 0x12, action);
		menuTxtData.play (&menuTxt1Id);
		break;
	case 1:
		menuTxtData.create ("AET_PS4_GALLERY_MAIN", "menu_txt_02", 0x12, action);
		menuTxtData.play (&menuTxt2Id);
		break;
	case 2:
		menuTxtData.create ("AET_PS4_GALLERY_MAIN", "menu_txt_03", 0x12, action);
		menuTxtData.play (&menuTxt3Id);
		break;
	case 3:
		menuTxtData.create ("AET_PS4_GALLERY_MAIN", "menu_txt_04", 0x12, action);
		menuTxtData.play (&menuTxt4Id);
		break;
	case 4:
		menuTxtData.create ("AET_PS4_GALLERY_MAIN", "menu_txt_05", 0x12, action);
		menuTxtData.play (&menuTxt5Id);
		break;
	}
	AetLayerArgs menuTxtBaseData ("AET_PS4_GALLERY_MAIN", "menu_txt_base", 0x12, action);
	menuTxtBaseData.play (&menuTxtBaseId);
}

// Fixes gallery not properly exiting and text boxes not working
i32 previousButton = 5;
bool
CsGalleryLoop (u64 This) {
	i32 state          = *(i32 *)(This + 0x68);
	i32 selectedButton = *(i32 *)(This + 0x70);
	if (state == 3 && previousButton != selectedButton) {
		playGalleryTxt (selectedButton, AetAction::IN_LOOP);
		if (previousButton != 5) playGalleryTxt (previousButton, AetAction::OUT_ONCE);
		previousButton = selectedButton;
	} else if (state == 6) {
		*(i32 *)(This + 0x68)   = 14;
		*(i32 *)(This + 0x6C)   = 1;
		*(i32 *)(This + 0x4598) = 5;
		previousButton          = 5;
		playGalleryTxt (selectedButton, AetAction::OUT_ONCE);
	} else if (state == 4) {
		previousButton = 5;
		playGalleryTxt (selectedButton, AetAction::OUT_ONCE);
	}

	return false;
}

// Fixes gallery photos
HOOK (void, LoadAndPlayAet, 0x1401AF0E0, diva::AetLayerArgs *args, AetAction action) {
	args->create ("AET_PS4_GALLERY_MAIN", args->layerName, args->priority, action);
	args->play (&args->unk_0x15C);
}

HOOK (void, PlayStatBase, 0x1401D4120, u64 a1, i32 a2) {
	originalPlayStatBase (a1, a2);
	if (theme != 3) return;
	auto str = (string *)(a1 + 0x360);
	if (strcmp (str->c_str (), "stat_base_ft") == 0) *str = "stat_base_dx";
	else if (strcmp (str->c_str (), "stat_base_up_ft") == 0) *str = "stat_base_up_dx";
	else if (strcmp (str->c_str (), "stat_base_down_ft") == 0) *str = "stat_base_down_dx";
}

void
init () {
	diva::taskAddition addition;
	addition.loop = CsGalleryLoop;
	diva::addTaskAddition ("CS_GALLERY", addition);

	INSTALL_HOOK (LoadAndPlayAet);
	INSTALL_HOOK (PlayStatBase);
}
} // namespace gallery
