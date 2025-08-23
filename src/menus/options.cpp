#include "diva.h"

namespace options {
using namespace diva;
i32 menuTxt1Id = 0;
i32 menuTxt2Id = 0;
i32 menuTxt3Id = 0;
i32 menuTxt4Id = 0;
i32 menuTxt5Id = 0;
i32 menuTxt6Id = 0;
i32 footerId;
char *footerName        = (char *)calloc (32, sizeof (char));
i32 optionMenuTopId     = 0;
InputType previousInput = InputType::UNKNOWN;
u8 previousButton       = 0;
u8 previousSubMenu      = 0;

void
playMenuTxt (u8 button, u8 subMenu, AetAction action) {
	const char *name;
	i32 *id;
	switch (button) {
	case 0:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_sound"; break;
		case 1: name = "option_menu_sound_txt_audio_device"; break;
		case 2: name = "option_menu_display_txt_target"; break;
		case 3: name = "option_menu_graphic_txt_graphic"; break;
		default: name = ""; break;
		}
		id = &menuTxt1Id;
		break;
	case 1:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_display"; break;
		case 1: name = "option_menu_sound_txt_theme"; break;
		case 2: name = "option_menu_display_txt_mode"; break;
		case 3: name = "option_menu_graphic_txt_image"; break;
		default: name = ""; break;
		}
		id = &menuTxt2Id;
		break;
	case 2:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_graphic"; break;
		case 1: name = "option_menu_sound_txt_bgm"; break;
		case 2: name = "option_menu_display_txt_resolution"; break;
		case 3: name = "option_menu_graphic_txt_aa"; break;
		default: name = ""; break;
		}
		id = &menuTxt3Id;
		break;
	case 3:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_npr"; break;
		case 1: name = "option_menu_sound_txt_btn"; break;
		case 3: name = "option_menu_graphic_txt_shadow"; break;
		default: name = ""; break;
		}
		id = &menuTxt4Id;
		break;
	case 4:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_autosave"; break;
		case 1: name = "option_menu_sound_txt_se"; break;
		case 3: name = "option_menu_graphic_txt_reflection"; break;
		default: name = ""; break;
		}
		id = &menuTxt5Id;
		break;
	case 5:
		switch (subMenu) {
		case 0: name = "option_top_menu_txt_save"; break;
		default: name = ""; break;
		}
		id = &menuTxt6Id;
		break;
	}
	AetLayerArgs menuTxtData ("AET_NSWGAM_OPTION_MAIN", name, 13, action);
	menuTxtData.play (id);
}

bool
OptionMenuSwitchInit (u64 task) {
	AetLayerArgs optionMenuTopData ("AET_NSWGAM_OPTION_MAIN", "option_top_menu loop", 7, AetAction::NONE);
	optionMenuTopData.play (&optionMenuTopId);

	InputType input = diva::getInputType ();
	previousInput   = input;
	sprintf (footerName, "footer_button_01_%02d", (i32)input);

	AetLayerArgs footerData ("AET_NSWGAM_OPTION_MAIN", footerName, 13, AetAction::NONE);
	footerData.play (&footerId);

	playMenuTxt (0, 0, AetAction::IN_LOOP);
	return false;
}

bool inited = false;
bool
OptionMenuSwitchLoop (u64 task) {
	if (*(u8 *)(task + 0xB0) != 0 && !inited) {
		OptionMenuSwitchInit (task);
		inited = true;
	}
	u8 subMenu      = *(u8 *)(task + 0xB3);
	subMenu         = subMenu > 1 ? subMenu - 1 : subMenu;
	u8 button       = *(u8 *)(task + 0xB4 + subMenu);
	InputType input = diva::getInputType ();
	if (input != previousInput || previousSubMenu != subMenu) {
		sprintf (footerName, "footer_button_%02d_%02d", (bool)subMenu + 1, (i32)input);
		AetLayerArgs footerData ("AET_NSWGAM_OPTION_MAIN", "option_top_menu loop", 13, AetAction::NONE);
		footerData.play (&footerId);
		previousInput = input;
	}
	if (button != previousButton || previousSubMenu != subMenu) {
		playMenuTxt (previousButton, previousSubMenu, AetAction::OUT_ONCE);
		playMenuTxt (button, subMenu, AetAction::IN_LOOP);
		previousButton  = button;
		previousSubMenu = subMenu;
	}
	return false;
}

bool
OptionMenuSwitchDestroy (u64 task) {
	diva::StopAet (&optionMenuTopId);
	diva::StopAet (&menuTxt1Id);
	diva::StopAet (&menuTxt2Id);
	diva::StopAet (&menuTxt3Id);
	diva::StopAet (&menuTxt4Id);
	diva::StopAet (&menuTxt5Id);
	diva::StopAet (&menuTxt6Id);
	diva::StopAet (&footerId);
	inited = false;
	return false;
}

void
init () {
	taskAddition addition;
	addition.init    = OptionMenuSwitchInit;
	addition.loop    = OptionMenuSwitchLoop;
	addition.destroy = OptionMenuSwitchDestroy;
	addTaskAddition ("OPTION_MENU_SWITCH", addition);
}
} // namespace options
