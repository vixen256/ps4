#include "diva.h"
#include "menus.h"

namespace customize {
using namespace diva;
i32 footerButtonId = 0;
i32 optionTxt0Id   = 0;
i32 optionTxt1Id   = 0;
i32 optionTxt2Id   = 0;
i32 optionTxt3Id   = 0;
i32 optionTxt4Id   = 0;
i32 optionTxt5Id   = 0;
i32 optionTxt6Id   = 0;
i32 optionTxt7Id   = 0;
i32 optionTxt8Id   = 0;
i32 soundListInId  = 0;
i32 currentMenu    = -1;
i32 previousOption = -1;
InputType previousInputType;
i32 gameOptionsArrowsUpId   = 0;
i32 gameOptionsArrowsDownId = 0;

i32 mdlPlateIndex[10]  = {0};
i32 mdlIconIndex[8][6] = {{0}};
i32 charaListId        = 0;

struct PlayCustomizeSelFooterArgs {
	string footerName;
	i32 screen;
};

// Fixes the header/footer being present on customize
HOOK (bool, CustomizeSelInit, 0x140687D10, u64 This) {
	auto cmnMenu       = (Task *)(0x14114C370);
	cmnMenu->state     = TaskState::HIDDEN;
	cmnMenu->nextState = TaskState::HIDDEN;
	if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x6C))) layer.value ()->color.w = 0.0;
	if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x70))) layer.value ()->color.w = 0.0;
	pvSel::hide ();
	return originalCustomizeSelInit (This);
}

// Fixes switching to customize from playlists
HOOK (bool, CustomizeSelIsLoaded, 0x140687CD0) {
	if (*(i32 *)0x14CC6F118 == 1) {
		if (implOfCustomizeSelInit (0x14CC6F100)) {
			*(i32 *)0x14CC6F118 = 2;
			*(i32 *)0x14CC6F124 = 2;
		}
	}

	return originalCustomizeSelIsLoaded ();
}

bool
CustomizeSelLoop (u64 task) {
	InputType input = getInputType ();
	if (currentMenu != -1 && input != previousInputType) {
		previousInputType = input;
		char buf[128];
		sprintf (buf, "footer_button_%02d_%02d", currentMenu + 1, (i32)input);
		AetLayerArgs layer ("AET_NSWGAM_CUSTOM_MAIN", buf, 26, AetAction::NONE);
		layer.play (&footerButtonId);
	}
	return false;
}

bool
CustomizeSelDestroy (u64 task) {
	StopAet (&footerButtonId);
	currentMenu  = -1;
	auto cmnMenu = (Task *)(0x14114C370);
	if (cmnMenu->state != TaskState::RUNNING) {
		cmnMenu->request = TaskRequest::RUN;
		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x6C))) layer.value ()->color.w = 1.0;
		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x70))) layer.value ()->color.w = 1.0;
	}
	u64 pvLoadData                = GetPvLoadData ();
	*(i32 *)(pvLoadData + 0x1D08) = -1;
	pvSel::unhide ();
	return false;
}

HOOK (void, PlayCustomizeSelFooter, 0x15F9811D0, void *a1, PlayCustomizeSelFooterArgs *args) {
	char buf[128];
	sprintf (buf, "footer_button_%02d_%02d", args->screen + 1, (i32)getInputType ());
	AetLayerArgs layer ("AET_NSWGAM_CUSTOM_MAIN", buf, 26, AetAction::NONE);
	layer.play (&footerButtonId);
	currentMenu       = args->screen;
	previousInputType = getInputType ();
	originalPlayCustomizeSelFooter (a1, args);
}

HOOK (void, StopCustomizeSelFooter, 0x140684A00, void *a1) {
	StopAet (&footerButtonId);
	originalStopCustomizeSelFooter (a1);
}

HOOK (void, PlayTshirtEditFooter, 0x140710680, void *a1, i32 index) {
	i32 realIndex = 0;
	if (index == 0) realIndex = 3;
	else if (index == 1) realIndex = 4;
	else if (index == 2) realIndex = 7;

	char buf[128];
	sprintf (buf, "footer_button_%02d_%02d", realIndex + 1, (i32)getInputType ());
	AetLayerArgs layer ("AET_NSWGAM_CUSTOM_MAIN", buf, 0x11, AetAction::NONE);
	layer.play (&footerButtonId);
	currentMenu       = realIndex;
	previousInputType = getInputType ();

	originalPlayTshirtEditFooter (a1, index);
}

void
playOptionText (i32 option, AetAction action) {
	const char *txtLayer = "";
	i32 *txtId           = nullptr;
	switch (option) {
	case 0:
		txtLayer = "game_menu_txt_button_config";
		txtId    = &optionTxt0Id;
		break;
	case 1:
		txtLayer = "game_menu_txt_multipress";
		txtId    = &optionTxt1Id;
		break;
	case 2:
		txtLayer = "game_menu_txt_ac_controller";
		txtId    = &optionTxt2Id;
		break;
	case 3:
		txtLayer = "game_menu_txt_vibration";
		txtId    = &optionTxt3Id;
		break;
	case 4:
		txtLayer = "game_menu_txt_vocal";
		txtId    = &optionTxt4Id;
		break;
	case 5:
		txtLayer = "game_menu_txt_icon";
		txtId    = &optionTxt5Id;
		break;
	case 6:
		txtLayer = "game_menu_txt_threshold";
		txtId    = &optionTxt6Id;
		break;
	case 7:
		txtLayer = "game_menu_txt_lag_config";
		txtId    = &optionTxt7Id;
		break;
	case 8:
		txtLayer = "game_menu_txt_shared_config";
		txtId    = &optionTxt8Id;
		break;
	}
	AetLayerArgs txtArgs ("AET_NSWGAM_CUSTOM_MAIN", txtLayer, 0xD, action);
	txtArgs.play (txtId);
}

HOOK (void *, GameOptionsLoop, 0x14066E0E0, u64 a1, i32 a2, bool a3) {
	if (a2 == 0) {
		StopAet (&optionTxt0Id);
		StopAet (&optionTxt1Id);
		StopAet (&optionTxt2Id);
		StopAet (&optionTxt3Id);
		StopAet (&optionTxt4Id);
		StopAet (&optionTxt5Id);
		StopAet (&optionTxt6Id);
		StopAet (&optionTxt7Id);
		StopAet (&optionTxt8Id);
		StopAet (&gameOptionsArrowsUpId);
		StopAet (&gameOptionsArrowsDownId);
		previousOption = -1;
	} else if (a2 == 1) {
		AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_up", 0x10, AetAction::IN_LOOP);
		args.play (&gameOptionsArrowsUpId);
		AetLayerArgs bottomArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_down", 0x10, AetAction::IN_LOOP);
		bottomArgs.play (&gameOptionsArrowsDownId);
	} else {
		i32 selectedOption = *(i32 *)(a1 + 0x60);
		if (selectedOption != previousOption) {
			if (previousOption != -1) playOptionText (previousOption, AetAction::OUT_ONCE);
			playOptionText (selectedOption, AetAction::IN_LOOP);

			if (previousOption != -1) {
				if (selectedOption == 0) {
					if (auto layer = aets->find (gameOptionsArrowsUpId)) {
						AetLayerArgs topArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_up", 0x10, AetAction::IN_ONCE);
						topArgs.play (&gameOptionsArrowsUpId);
						AetLayerArgs bottomArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_down", 0x10, AetAction::SPECIAL_LOOP);
						bottomArgs.play (&gameOptionsArrowsDownId);
					}
				} else if (selectedOption == 8) {
					if (auto layer = aets->find (gameOptionsArrowsDownId)) {
						AetLayerArgs topArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_up", 0x10, AetAction::SPECIAL_LOOP);
						topArgs.play (&gameOptionsArrowsUpId);
						AetLayerArgs bottomArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_down", 0x10, AetAction::IN_ONCE);
						bottomArgs.play (&gameOptionsArrowsDownId);
					}
				} else if (previousOption > selectedOption) {
					AetLayerArgs topArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_up", 0x10, AetAction::SPECIAL_LOOP);
					topArgs.play (&gameOptionsArrowsUpId);
					AetLayerArgs bottomArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_down", 0x10, AetAction::LOOP);
					bottomArgs.play (&gameOptionsArrowsDownId);
				} else {
					AetLayerArgs topArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_up", 0x10, AetAction::LOOP);
					topArgs.play (&gameOptionsArrowsUpId);
					AetLayerArgs bottomArgs ("AET_NSWGAM_CUSTOM_MAIN", "setting_menu_bg_arrow_down", 0x10, AetAction::SPECIAL_LOOP);
					bottomArgs.play (&gameOptionsArrowsDownId);
				}
			}

			previousOption = selectedOption;
		}
	}
	return originalGameOptionsLoop (a1, a2, a3);
}

HOOK (void, ButtonFxListIn, 0x1406985B0, u64 a1) {
	AetLayerArgs soundListInArgs ("AET_NSWGAM_CUSTOM_MAIN", "sound_list_in", 0xD, AetAction::NONE);
	soundListInArgs.play (&soundListInId);
	originalButtonFxListIn (a1);
}

HOOK (void, ButtonFxUnload, 0x1406996C0, u64 a1) {
	StopAet (&soundListInId);
	originalButtonFxUnload (a1);
}

HOOK (u32 *, SetCursorColor, 0x14065E410, void *a1, u32 *rgbaColor) {
	originalSetCursorColor (a1, rgbaColor);
	*rgbaColor |= 0xFF;
	return rgbaColor;
}

i32 choiceListPackId[18] = {0};

extern "C" {
void
LoadChoiceListStatus (AetLayoutData *placeholder, ModuleData *module, i32 index, bool selected, i32 moveState) {
	bool centerModule = (moveState == 1) ? index == 9 : index == 8;
	const char *name;
	if (selected) {
		if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone) ||
		    (module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == 0) {
			name = centerModule ? "choice_list_status_set_sel_etc" : "choice_list_status_set_etc";
		} else if (module->attr & ModuleAttr::FutureSound) {
			name = centerModule ? "choice_list_status_set_sel_f" : "choice_list_status_set_f";
		} else {
			name = centerModule ? "choice_list_status_set_sel_t" : "choice_list_status_set_t";
		}
	} else if (module->id < 0) {
		if ((moveState == 1) ? index != 9 : index != 8) {
			StopAet (&choiceListPackId[index]);
			return;
		}
		name = "choice_list_status_unset_sel_etc";
	} else if (CheckModuleUnlocked (FindModule (GetSaveData (), module->id))) {
		if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone) ||
		    (module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == 0) {
			name = centerModule ? "choice_list_status_have_sel_etc" : "choice_list_status_have_etc";
		} else if (module->attr & ModuleAttr::FutureSound) {
			name = centerModule ? "choice_list_status_have_sel_f" : "choice_list_status_have_f";
		} else {
			name = centerModule ? "choice_list_status_have_sel_t" : "choice_list_status_have_t";
		}
	} else if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone) ||
	           (module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == 0) {
		name = centerModule ? "choice_list_status_none_sel_etc" : "choice_list_status_none_etc";
	} else if (module->attr & ModuleAttr::FutureSound) {
		name = centerModule ? "choice_list_status_none_sel_f" : "choice_list_status_none_f";
	} else {
		name = centerModule ? "choice_list_status_none_sel_t" : "choice_list_status_none_t";
	}

	AetLayerArgs args;
	if (moveState == 1 || moveState == 2) index -= 1;
	auto priority = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;
	if (moveState == 1 || moveState == 2) index += 1;

	args.create ("AET_NSWGAM_CUSTOM_MAIN", name, priority, AetAction::NONE);
	args.position = placeholder->position;
	args.scale.x  = placeholder->matrix.x.x;
	args.scale.y  = placeholder->matrix.y.y;
	args.color.w  = placeholder->opacity;
	args.play (&choiceListPackId[index]);
}

i32 lastMoveState = -1;

HOOK (void, LoadModuleChoiceList, 0x140691D47);
const char *
realLoadModuleChoiceList (u64 This, i32 moduleId, i32 index) {
	i32 moveState = *(i32 *)(This + 0x1BC);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState     = moveState;
	bool centerModule = (moveState == 1) ? index == 9 : index == 8;

	if (moduleId == -1) {
		StopAet (&choiceListPackId[index]);
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}
	auto modules   = (vector<ModuleData *> *)(This + 0x70);
	auto moduleOpt = modules->at (moduleId);
	if (!moduleOpt.has_value () || **moduleOpt == nullptr) {
		StopAet (&choiceListPackId[index]);
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}
	auto module = **moduleOpt;

	char buf[64];
	sprintf (buf, "p_choice_mdl_status%02d_c", (moveState == 2 || moveState == 1) ? index : index + 1);
	auto comp = (AetComposition *)(This + 0x1D0);
	if (auto placeholder = comp->find (string (buf))) LoadChoiceListStatus (placeholder.value (), module, index, *(i32 *)(This + 0x2C) == moduleId, moveState);

	if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone))
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	else if (module->attr & ModuleAttr::FutureSound) return centerModule ? "choice_list_mdl_base_f_sel" : "choice_list_mdl_base_f";
	else if (module->attr & ModuleAttr::ColorfulTone) return centerModule ? "choice_list_mdl_base_t_sel" : "choice_list_mdl_base_t";
	else return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
}

HOOK (void, LoadHairstyleChoiceList, 0x1406892F8);
const char *
realLoadHairstyleChoiceList (u64 This, i32 hairstyleId, i32 index) {
	i32 moveState = *(i32 *)(This + 0x1E4);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState     = moveState;
	bool centerModule = (moveState == 1) ? index == 9 : index == 8;

	if (hairstyleId == -1) {
		StopAet (&choiceListPackId[index]);
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}
	auto taskData = *(u64 *)0x14CC6F178;
	auto modules  = (vector<ModuleData> *)(taskData + 0x1A0);

	auto hairstyles   = (vector<CustomizeItemData *> *)(This + 0x108);
	auto hairstyleOpt = hairstyles->at (hairstyleId);
	if (!hairstyleOpt.has_value ()) {
		StopAet (&choiceListPackId[index]);
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}
	auto hairstyle = **hairstyleOpt;
	if (hairstyle == nullptr) {
		printf ("Hairstyle with offset %d is NULL\n", hairstyleId);
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}

	char buf[64];
	sprintf (buf, "p_choice_mdl_status%02d_c", (moveState == 2 || moveState == 1) ? index : index + 1);
	auto comp = (AetComposition *)(This + 0x1F8);

	if (hairstyle->bind_module == -1) {
		if (auto placeholder = comp->find (string (buf))) {
			const char *name;
			if (*(i32 *)(This + 0x2C) == hairstyleId) {
				name = centerModule ? "choice_list_status_set_sel_etc" : "choice_list_status_set_etc";
			} else {
				if (index != 8) {
					StopAet (&choiceListPackId[index]);
					return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
				}
				name = "choice_list_status_unset_sel_etc";
			}

			AetLayerArgs args;
			if (moveState == 1 || moveState == 2) index -= 1;
			auto priority = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;
			if (moveState == 1 || moveState == 2) index += 1;

			args.create ("AET_NSWGAM_CUSTOM_MAIN", name, priority, AetAction::NONE);
			args.position = placeholder.value ()->position;
			args.scale.x  = placeholder.value ()->matrix.x.x;
			args.scale.y  = placeholder.value ()->matrix.y.y;
			args.color.w  = placeholder.value ()->opacity;
			args.play (&choiceListPackId[index]);
		}

		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}

	ModuleData *module = 0;
	for (auto it = modules->begin (); it != modules->end (); it++) {
		if (it == 0) continue;
		if (it->id == hairstyle->bind_module) {
			module = it;
			break;
		}
	}
	if (module == nullptr) {
		StopAet (&choiceListPackId[index]);
		printf ("Failed to find module %d for %s\n", hairstyle->bind_module, hairstyle->name.c_str ());
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	}

	if (auto placeholder = comp->find (string (buf))) LoadChoiceListStatus (placeholder.value (), module, index, *(i32 *)(This + 0x2C) == hairstyleId, moveState);

	if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone))
		return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
	else if (module->attr & ModuleAttr::FutureSound) return centerModule ? "choice_list_mdl_base_f_sel" : "choice_list_mdl_base_f";
	else if (module->attr & ModuleAttr::ColorfulTone) return centerModule ? "choice_list_mdl_base_t_sel" : "choice_list_mdl_base_t";
	else return centerModule ? "choice_list_mdl_base_etc_sel" : "choice_list_mdl_base_etc";
}

HOOK (void, LoadItemChoiceList, 0x14068D28B);
const char *
realLoadItemChoiceList (u64 This, i32 itemId, i32 index) {
	i32 moveState = *(i32 *)(This + 0x124);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState     = moveState;
	bool centerModule = (moveState == 1) ? index == 9 : index == 8;

	auto items   = (vector<CustomizeItemData *> *)(This + 0x48);
	auto itemOpt = items->at (itemId);
	if (!itemOpt.has_value () || **itemOpt == nullptr) {
		StopAet (&choiceListPackId[index]);
		return centerModule ? "choice_list_itm_base_sel" : "choice_list_itm_base";
	}
	auto item = **itemOpt;

	char buf[64];
	sprintf (buf, "p_choice_mdl_status%02d_c", (moveState == 2 || moveState == 1) ? index : index + 1);
	auto comp = (AetComposition *)(This + 0x138);
	if (auto placeholder = comp->find (string (buf))) {
		const char *name;
		if (*(i32 *)(This + 0x34) == itemId) name = (centerModule ? "choice_list_status_set_sel_etc" : "choice_list_status_set_etc");
		else if (CheckModuleUnlocked (FindCstmItem (GetSaveData (), item->id))) name = (centerModule ? "choice_list_status_have_sel_etc" : "choice_list_status_have_etc");
		else name = (centerModule ? "choice_list_status_none_sel_etc" : "choice_list_status_none_etc");

		AetLayerArgs args;
		if (moveState == 1 || moveState == 2) index -= 1;
		auto priority = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;
		if (moveState == 1 || moveState == 2) index += 1;

		args.create ("AET_NSWGAM_CUSTOM_MAIN", name, priority, AetAction::NONE);
		args.position = placeholder.value ()->position;
		args.scale.x  = placeholder.value ()->matrix.x.x;
		args.scale.y  = placeholder.value ()->matrix.y.y;
		args.color.w  = placeholder.value ()->opacity;
		args.play (&choiceListPackId[index]);
	}

	return centerModule ? "choice_list_itm_base_sel" : "choice_list_itm_base";
}

HOOK (void, SetModuleSprArgs, 0x140692C73);
void
realSetModuleSprArgs (u64 This, SprArgs *args, i32 index) {
	i32 moveState = *(i32 *)(This + 0x1BC);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState = moveState;
	if (moveState == 1 || moveState == 2) index -= 1;
	args->layer = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;

	auto comp = (AetComposition *)(This + 0x1D0);

	char buf[64];
	sprintf (buf, "p_choice_mdl_pic%02d_c", index + 1);

	if (auto placeholder = comp->find (string (buf))) {
		args->trans    = placeholder.value ()->position;
		args->scale.x  = placeholder.value ()->matrix.x.x;
		args->scale.y  = placeholder.value ()->matrix.y.y;
		args->color[3] = placeholder.value ()->opacity * 255.0;
	}
}

HOOK (void, SetHairstyleSprArgs, 0x140689F89);
void
realSetHairstyleSprArgs (u64 This, SprArgs *args, i32 index) {
	i32 moveState = *(i32 *)(This + 0x1E4);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState = moveState;
	if (moveState == 1 || moveState == 2) index -= 1;
	args->layer = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;

	auto comp = (AetComposition *)(This + 0x1F8);

	char buf[64];
	sprintf (buf, "p_choice_mdl_pic%02d_c", index + 1);

	if (auto placeholder = comp->find (string (buf))) {
		args->trans    = placeholder.value ()->position;
		args->scale.x  = placeholder.value ()->matrix.x.x;
		args->scale.y  = placeholder.value ()->matrix.y.y;
		args->color[3] = placeholder.value ()->opacity * 255.0;
	}
}

HOOK (void, SetItemSprArgs, 0x14068DE2D);
void
realSetItemSprArgs (u64 This, SprArgs *args, i32 index) {
	i32 moveState = *(i32 *)(This + 0x124);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState = moveState;
	if (moveState == 1 || moveState == 2) index -= 1;
	args->layer = ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5;

	auto comp = (AetComposition *)(This + 0x138);

	char buf[64];
	sprintf (buf, "p_choice_mdl_pic%02d_c", index + 1);

	if (auto placeholder = comp->find (string (buf))) {
		args->trans    = placeholder.value ()->position;
		args->scale.x  = placeholder.value ()->matrix.x.x;
		args->scale.y  = placeholder.value ()->matrix.y.y;
		args->color[3] = placeholder.value ()->opacity * 255.0;
	}
}

HOOK (void, LoadReccomendChoiceList, 0x14069237E);
const char *
realLoadReccomendChoiceList (u64 This, i32 index) {
	i32 moveState = *(i32 *)(This + 0x1BC);
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState     = moveState;
	bool centerModule = (moveState == 1) ? index == 9 : index == 8;

	return centerModule ? "choice_list_recommend_sel" : "choice_list_recommend";
}

HOOK (void, SetModuleChoiceListPriority, 0x140691DC4);
HOOK (void, SetHairstyleChoiceListPriority, 0x140689375);
HOOK (void, SetItemChoiceListPriority, 0x14068D2FF);
i32
realSetChoiceListPriority (i32 moveState, i32 index) {
	if (moveState == 0) moveState = lastMoveState;
	lastMoveState = moveState;
	if (moveState == 1 || moveState == 2) index -= 1;
	return ((index > (moveState == 2 ? 7 : 8) ? (index * -1) + 17 : (index + 2)) * 2) + 5 - 1;
}

HOOK (void, Memset, 0x14097B0E0);
}

HOOK (void, DestroyModuleSelect, 0x1406910D0, u64 This) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	originalDestroyModuleSelect (This);
}

HOOK (void, DestroyHairstyleSelect, 0x140688550, u64 This) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	originalDestroyHairstyleSelect (This);
}

HOOK (void, DestroyItemSelect, 0x14068C5D0, u64 This) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	originalDestroyItemSelect (This);
}

HOOK (u64, ModulePreviewInit, 0x1406962E0, u64 a1) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);

	for (i32 i = 0; i < 10; i++)
		StopAet (&mdlPlateIndex[i]);
	for (i32 i = 0; i < 8; i++)
		for (i32 j = 0; j < 6; j++)
			StopAet (&mdlIconIndex[i][j]);
	StopAet (&charaListId);

	return originalModulePreviewInit (a1);
}

HOOK (u64, HairstylePreviewInit, 0x14068BC90, u64 a1) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	return originalHairstylePreviewInit (a1);
}

HOOK (u64, ItemPreviewInit, 0x14068F9B0, u64 a1) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	return originalItemPreviewInit (a1);
}

extern "C" {
HOOK (void, UpdateBG10SpriteColor, 0x14060D60D);
HOOK (void, UpdateBG10TextColor, 0x14060D999);
f32
UpdateBG10Color () {
	auto args = (AetLayerArgs *)0x14CC07620;
	AetComposition comp;
	GetComposition (&comp, args->id);
	if (auto layer = comp.find (string ("popup_txt"))) {
		auto opacity = layer.value ()->opacity;
		// Update button opacity
		auto noArgs = (AetLayerArgs *)0x14CC07818;
		if (auto noLayer = aets->find (noArgs->id)) noLayer.value ()->color.w = opacity;
		auto yesArgs = (AetLayerArgs *)0x14CC07A10;
		if (auto yesLayer = aets->find (yesArgs->id)) yesLayer.value ()->color.w = opacity;

		return opacity;
	}
	return 1.0;
}
HOOK (void, UpdateBg05TextColor, 0x14060DB43);
f32
UpdateBG05Color () {
	auto args = (AetLayerArgs *)0x14CC07428;
	AetComposition comp;
	GetComposition (&comp, args->id);
	if (auto layer = comp.find (string ("popup_txt"))) return layer.value ()->opacity;
	return 1.0;
}
}

const char *characters[] = {"mdl_plate_mei", "mdl_plate_other", "mdl_plate_rand", "mdl_plate_mik", "mdl_plate_rin", "mdl_plate_len", "mdl_plate_luk", "mdl_plate_kai"};
i32 characterOffsets[]   = {3, 2, 1, 0, 7, 6, 5, 4};
bool inited              = false;
i32 oldChara             = 0;
HOOK (void, DrawMdlPlate, 0x140693830, u64 a1, i32 a2, u8 isIn) {
	if (!isIn) {
		for (i32 i = 0; i < 10; i++)
			StopAet (&mdlPlateIndex[i]);

		StopAet (&charaListId);
		return;
	}

	if (a1 == 0) return;
	auto data = *(u64 *)(a1 + 0x08);
	if (data == 0) return;
	i32 selectedChara = *(i32 *)(data + 0x40);

	const char *layerName;
	if (oldChara == 7 && selectedChara == 0) layerName = "mdl_chara_list_up";
	else if (oldChara == 0 && selectedChara == 7) layerName = "mdl_chara_list_down";
	else if (oldChara < selectedChara) layerName = "mdl_chara_list_up";
	else if (oldChara > selectedChara) layerName = "mdl_chara_list_down";
	else if (*(i32 *)(a1 + 0x20) == 0) layerName = "mdl_chara_list_in";
	else layerName = "mdl_chara_list_loop";

	AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", layerName, 14, AetAction::NONE);
	args.play (&charaListId);

	oldChara = selectedChara;

	AetComposition comp;
	GetComposition (&comp, charaListId);

	if (oldChara != selectedChara) {
		if (auto layout = comp.find (string ("p_chara_list00_c"))) {
			auto charaIndex = -selectedChara + 7;
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], 14, AetAction::IN_ONCE);
			args.position = layout.value ()->position;
			args.color.w  = layout.value ()->opacity;
			args.play (&mdlPlateIndex[0]);
		}
		if (auto layout = comp.find (string ("p_chara_list08_c"))) {
			auto charaIndex = selectedChara + 8;
			while (charaIndex > 7)
				charaIndex -= 8;
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], 14, AetAction::IN_ONCE);
			args.position = layout.value ()->position;
			args.color.w  = layout.value ()->opacity;
			args.play (&mdlPlateIndex[9]);
		}
	}

	inited = true;

	i32 charaIndex = selectedChara;
	for (i32 i = 0; i < 8; i++) {
		if (charaIndex > 7) charaIndex = 0;
		char placeholderName[64];
		sprintf (placeholderName, "p_chara_list%02d_c", i + 1);
		if (auto layout = comp.find (string (placeholderName))) {
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], i == 3 ? 15 : 14, i == 3 ? AetAction::LOOP : AetAction::IN_ONCE);
			args.position = layout.value ()->position;
			args.color.w  = layout.value ()->opacity;
			args.play (&mdlPlateIndex[i + 1]);
		}
		charaIndex++;
	}
}

HOOK (void, DrawMdlIcon, 0x1406940F0, u64 a1, i32 a2, u8 isIn) {
	for (i32 i = 0; i < 8; i++)
		for (i32 j = 0; j < 6; j++)
			StopAet (&mdlIconIndex[i][j]);
	if (!isIn) {
		inited = false;
		return;
	}

	if (a1 == 0) return;
	auto data = *(u64 *)(a1 + 0x08);
	if (data == 0) return;
	i32 selectedChara = *(i32 *)(data + 0x40);

	AetComposition comp;
	GetComposition (&comp, charaListId);

	auto pvId = **(i32 **)(data + 0x18);
	if (auto entry = getPvDbEntry (pvId)) {
		auto charaGuest = (i32 *)(data + 0xB0);
		auto charaNo    = (i32 *)(data + 0xC8);
		for (size_t i = 0; i < entry.value ()->performers.length (); i++) {
			if (entry.value ()->performers.at (i).value ()->type != 0 && entry.value ()->performers.at (i).value ()->type != 6) continue;
			auto charaListIndex = entry.value ()->performers.at (i).value ()->chara;
			if (charaListIndex == -1) charaListIndex = 0;
			if (charaListIndex > 5) charaListIndex -= 2;
			charaListIndex += characterOffsets[selectedChara];
			while (charaListIndex > 7)
				charaListIndex -= 8;
			bool isGuest = charaGuest[i] == 1;
			char placeholderName[64];
			sprintf (placeholderName, "p_chara_list%02d_c", charaListIndex + 1);
			if (auto layout = comp.find (string (placeholderName))) {
				AetComposition offsetComp;
				GetComposition (&offsetComp, mdlPlateIndex[charaListIndex + 1]);
				if (auto layoutOffset = offsetComp.find (string ("p_chara_list_part_c"))) {
					char buf[64];
					sprintf (buf, "mdl_icon_part_%c%02d", isGuest ? 'g' : 'v', charaNo[i]);

					i32 j = 0;
					for (; j < 6; j++)
						if (mdlIconIndex[charaListIndex][j] == 0) break;

					AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", buf, 16, AetAction::NONE);
					args.position.x = layout.value ()->position.x + layoutOffset.value ()->position.x + (layoutOffset.value ()->width / 1.5 * j);
					args.position.y = layout.value ()->position.y + layoutOffset.value ()->position.y;
					args.color.w    = layout.value ()->opacity;
					args.play (&mdlIconIndex[charaListIndex][j]);
				}
			}
		}
	}
}

HOOK (void, DisplayMdl, 0x1406947B0, u64 a1) {
	if (a1 == 0) return;

	if (auto aet = aets->find (charaListId)) {
		if (aet.value ()->currentFrame >= aet.value ()->layer->endTime - 1 && strcmp (aet.value ()->layer->name, "mdl_chara_list_loop") != 0) {
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", "mdl_chara_list_loop", 14, AetAction::NONE);
			args.play (&charaListId);
		}
	}

	auto data = *(u64 *)(a1 + 0x08);
	if (data == 0) return;
	i32 selectedChara = *(i32 *)(data + 0x40);

	AetComposition comp;
	GetComposition (&comp, charaListId);

	for (i32 i = 0; i < 10; i++) {
		char placeholderName[64];
		sprintf (placeholderName, "p_chara_list%02d_c", i);
		if (auto layer = aets->find (mdlPlateIndex[i])) {
			if (auto layout = comp.find (string (placeholderName))) {
				layer.value ()->position = layout.value ()->position;
				layer.value ()->color.w  = layout.value ()->opacity;
			}
		}
	}

	auto pvId = **(i32 **)(data + 0x18);
	if (auto entry = getPvDbEntry (pvId)) {
		for (size_t i = 0; i < entry.value ()->performers.length (); i++) {
			auto charaListIndex = entry.value ()->performers.at (i).value ()->chara;
			if (charaListIndex == -1) charaListIndex = 0;
			if (charaListIndex > 5) charaListIndex -= 2;
			charaListIndex += characterOffsets[selectedChara];
			while (charaListIndex > 7)
				charaListIndex -= 8;
			char placeholderName[64];
			sprintf (placeholderName, "p_chara_list%02d_c", charaListIndex + 1);
			for (auto j = 0; j < 6; j++) {
				if (mdlIconIndex[charaListIndex][j] == 0) break;
				if (auto layer = aets->find (mdlIconIndex[charaListIndex][j])) {
					if (auto layout = comp.find (string (placeholderName))) {
						AetComposition offsetComp;
						GetComposition (&offsetComp, mdlPlateIndex[charaListIndex + 1]);
						if (auto layoutOffset = offsetComp.find (string ("p_chara_list_part_c"))) {
							layer.value ()->position.x = layout.value ()->position.x + layoutOffset.value ()->position.x + (layoutOffset.value ()->width / 1.5 * j);
							layer.value ()->position.y = layout.value ()->position.y + layoutOffset.value ()->position.y;
							layer.value ()->color.w    = layout.value ()->opacity;
						}
					}
				}
			}
		}
	}
}

// Names and original func and values found by koren, 0x18CDE0 in CUSA06211
FUNCTION_PTR (void *, light_set_get_by_id, 0x1404326E0, i32 id);
FUNCTION_PTR (void, light_set_type, 0x155F73310, void *light_set, i32 type);
FUNCTION_PTR (void, light_set_ambient, 0x140432820, void *light_set, f32, f32, f32, f32);
FUNCTION_PTR (void, light_set_diffuse, 0x155422D40, void *light_set, f32, f32, f32, f32);
FUNCTION_PTR (void, light_set_specular, 0x140432900, void *light_set, f32, f32, f32, f32);
FUNCTION_PTR (void, light_set_position, 0x140432980, void *light_set, f32, f32, f32);
FUNCTION_PTR (void *, render_get, 0x14049F8D0);
FUNCTION_PTR (void, render_set_exposure, 0x1404A0490, void *render, f32);

HOOK (void, CustomizeSetLightingInfo, 0x14F621A00) {
	originalCustomizeSetLightingInfo ();

	void *set = light_set_get_by_id (0); // LIGHT_SET_MAIN
	light_set_type (set, 1);             // LIGHT_PARALLEL
	void *rend = render_get ();
	light_set_position (set, -0.2f, 0.39272901f, 0.70158201f);

	if (LoadLibrary ("FutureToneCustomization.dll")) {
		bool isFt = GetCurrentStyle () == 0;
		if (*(void **)0x14CC5EF18 != nullptr) isFt = *(u8 *)(*(u64 *)0x14CC5EF18 + 0x27538) == 0;

		if (isFt) {
			light_set_ambient (set, 0.07, 0.07, 0.07, 1.0);
			light_set_diffuse (set, 0.65, 0.65, 0.65, 1.0);
			light_set_specular (set, 0.8f, 0.8f, 0.8f, 0.8f);
			render_set_exposure (rend, 2.5);
		}
	}
}

void
init () {
	INSTALL_HOOK (CustomizeSelInit);
	INSTALL_HOOK (CustomizeSelIsLoaded);
	INSTALL_HOOK (PlayCustomizeSelFooter);
	INSTALL_HOOK (StopCustomizeSelFooter);
	INSTALL_HOOK (PlayTshirtEditFooter);
	INSTALL_HOOK (GameOptionsLoop);
	INSTALL_HOOK (ButtonFxListIn);
	INSTALL_HOOK (ButtonFxUnload);
	INSTALL_HOOK (SetCursorColor);

	INSTALL_HOOK (LoadModuleChoiceList);
	INSTALL_HOOK (LoadHairstyleChoiceList);
	INSTALL_HOOK (LoadItemChoiceList);
	INSTALL_HOOK (LoadReccomendChoiceList);

	INSTALL_HOOK (SetModuleSprArgs);
	INSTALL_HOOK (SetHairstyleSprArgs);
	INSTALL_HOOK (SetItemSprArgs);

	INSTALL_HOOK (SetModuleChoiceListPriority);
	INSTALL_HOOK (SetHairstyleChoiceListPriority);
	INSTALL_HOOK (SetItemChoiceListPriority);

	INSTALL_HOOK (Memset);

	INSTALL_HOOK (DestroyModuleSelect);
	INSTALL_HOOK (DestroyHairstyleSelect);
	INSTALL_HOOK (DestroyItemSelect);
	INSTALL_HOOK (ModulePreviewInit);
	INSTALL_HOOK (HairstylePreviewInit);
	INSTALL_HOOK (ItemPreviewInit);

	taskAddition addition;
	addition.loop    = CustomizeSelLoop;
	addition.destroy = CustomizeSelDestroy;
	addTaskAddition ("CustomizeSel", addition);

	// Use the right font
	WRITE_MEMORY (0x140692A3D, i8, 0x10); // Modules
	WRITE_MEMORY (0x140689C48, i8, 0x10); // Hairstyles
	WRITE_MEMORY (0x14068DC1D, i8, 0x10); // Accessories

	WRITE_MEMORY (0x140692D50, i32, 26); // Module name text priority
	WRITE_MEMORY (0x14068A05C, i32, 26); // Hairstyle name text priority
	WRITE_MEMORY (0x14068DEFC, i32, 26); // Item name text priority
	WRITE_MEMORY (0x1406930EE, i32, 26); // Module VP cost text priority
	WRITE_MEMORY (0x14068A457, i32, 26); // Hairstyle VP cost text priority
	WRITE_MEMORY (0x14068E2BE, i32, 26); // Item VP cost text priority
	WRITE_MEMORY (0x140692398, i32, 26); // Reccomended module priority

	WRITE_MEMORY (0x1406930BE, u8, 0x81, 0xE2, 0x00, 0x00, 0x00, 0xFF); // Module VP cost text colour
	WRITE_MEMORY (0x14068A427, u8, 0x81, 0xE2, 0x00, 0x00, 0x00, 0xFF); // Hairstyle VP cost text colour
	WRITE_MEMORY (0x14068E28E, u8, 0x81, 0xE2, 0x00, 0x00, 0x00, 0xFF); // Item VP cost text colour

	WRITE_NOP (0x1406923A3, 4);

	WRITE_NOP (0x14069223D, 4);
	WRITE_NOP (0x140689899, 4);

	WRITE_NOP (0x140691E27, 3); // SetModuleChoiceListPriority
	WRITE_NOP (0x1406893D7, 3); // SetHairstyleChoiceListPriority
	WRITE_NOP (0x14068D361, 3); // SetItemChoiceListPriority

	WRITE_MEMORY (0x140677FA9, i32, 26); // Choice_conf priority
	WRITE_MEMORY (0x140677E86, i32, 27); // Choice_conf button priority

	WRITE_MEMORY (0x14066375F, u8, 0x16); // Not enough VP base
	WRITE_MEMORY (0x15ED7435B, u8, 0x16); // Cannot change hairstyle base

	WRITE_MEMORY (0x140698306, u8, 0xC7, 0x45, 0xE0, 0x00, 0x00, 0x5C, 0x43, 0x90, 0x90, 0x90); // Squish FX text

	INSTALL_HOOK (UpdateBG10SpriteColor);
	INSTALL_HOOK (UpdateBG10TextColor);
	INSTALL_HOOK (UpdateBg05TextColor);

	WRITE_MEMORY (0x140696A23, u8, 0x48, 0x8B, 0x53, 0x18, 0xEB, 0xAB); // Some weird jank to start fade in 1 frame earlier

	INSTALL_HOOK (DrawMdlPlate);
	INSTALL_HOOK (DrawMdlIcon);
	INSTALL_HOOK (DisplayMdl);

	// Load 18 modules at once
	WRITE_MEMORY (0x140691739, i32, 17); // Modules
	WRITE_MEMORY (0x14068CB86, i32, 17); // Hairstyles
	WRITE_MEMORY (0x140688C19, i32, 17); // Items

	INSTALL_HOOK (CustomizeSetLightingInfo);

	WRITE_MEMORY (0x14067D5C3, u32, 0x10001); // Draw song name with border
}
} // namespace customize
