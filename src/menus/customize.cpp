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
		AetLayerArgs layer ("AET_NSWGAM_CUSTOM_MAIN", buf, 21, AetAction::NONE);
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
	AetLayerArgs layer ("AET_NSWGAM_CUSTOM_MAIN", buf, 21, AetAction::NONE);
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
	i32 *txtId;
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

i32 choiceListPackId[16] = {0};

extern "C" {
void
LoadChoiceListPlayTxt (AetLayoutData *layout, ModuleAttr attr, i32 index) {
	if ((attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) {
		StopAet (&choiceListPackId[index]);
	} else if (attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) {
		AetLayerArgs args;
		auto priority = index;
		if (index > 5) {
			priority = -priority;
			priority += 10;
		}
		priority *= 2;
		priority += 10;

		if (attr & ModuleAttr::FutureSound) args.create ("AET_NSWGAM_CUSTOM_MAIN", "choice_list_pack_f", priority, AetAction::NONE);
		else args.create ("AET_NSWGAM_CUSTOM_MAIN", "choice_list_pack_t", priority, AetAction::NONE);
		args.position = layout->position;
		args.color.w  = layout->opacity;
		if (index == 5) {
			args.scale = Vec3{1.2, 1.2, 1.2};
			args.position.y -= 5;
		} else if (index == 11) {
			args.color.w = 0.0;
		}
		args.play (&choiceListPackId[index]);
	} else {
		StopAet (&choiceListPackId[index]);
	}
}

HOOK (void, LoadModuleChoiceList, 0x140691D47);
const char *
realLoadModuleChoiceList (u64 This, i32 moduleId, i32 index) {
	if (moduleId == -1) {
		StopAet (&choiceListPackId[index]);
		return "choice_list_mdl_base_etc_sel";
	}
	auto modules   = (vector<ModuleData *> *)(This + 0x70);
	auto moduleOpt = modules->at (moduleId);
	if (!moduleOpt.has_value ()) {
		StopAet (&choiceListPackId[index]);
		return "choice_list_mdl_base_etc_sel";
	}
	auto module = **moduleOpt;
	if (module == 0) return "choice_list_mdl_base_etc_sel";

	auto layouts = *(u64 *)(This + 0x1C8);
	if (layouts == 0) return "choice_list_mdl_base_etc_sel";
	auto offset = *(i32 *)(This + 0x1AC);
	auto layout = *(AetLayoutData **)(layouts + ((index + offset) * 16));
	if (layout == 0) return "choice_list_mdl_base_etc_sel";

	LoadChoiceListPlayTxt (layout, module->attr, index);

	if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) return "choice_list_mdl_base_etc_sel";
	else if (module->attr & ModuleAttr::FutureSound) return "choice_list_mdl_base_f_sel";
	else if (module->attr & ModuleAttr::ColorfulTone) return "choice_list_mdl_base_t_sel";
	else return "choice_list_mdl_base_etc_sel";
}

HOOK (void, LoadHairstyleChoiceList, 0x1406892F8);
const char *
realLoadHairstyleChoiceList (u64 This, i32 hairstyleId, i32 index) {
	if (hairstyleId == -1) {
		StopAet (&choiceListPackId[index]);
		return "choice_list_mdl_base_etc_sel";
	}
	auto taskData = *(u64 *)0x14CC6F178;
	auto modules  = (vector<ModuleData> *)(taskData + 0x1A0);

	auto hairstyles   = (vector<CustomizeItemData *> *)(This + 0x108);
	auto hairstyleOpt = hairstyles->at (hairstyleId);
	if (!hairstyleOpt.has_value ()) {
		StopAet (&choiceListPackId[index]);
		return "choice_list_mdl_base_etc_sel";
	}
	auto hairstyle = **hairstyleOpt;
	if (hairstyle == 0) {
		printf ("Hairstyle with offset %d is NULL\n", hairstyleId);
		return "choice_list_mdl_base_etc_sel";
	}
	if (hairstyle->bind_module == -1) {
		StopAet (&choiceListPackId[index]);
		return "choice_list_mdl_base_etc_sel";
	}
	ModuleData *module = 0;
	for (auto it = modules->begin (); it != modules->end (); it++) {
		if (it == 0) continue;
		if (it->id == hairstyle->bind_module) {
			module = it;
			break;
		}
	}
	if (module == 0) {
		StopAet (&choiceListPackId[index]);
		printf ("Failed to find module %d for %s\n", hairstyle->bind_module, hairstyle->name.c_str ());
		return "choice_list_mdl_base_etc_sel";
	}

	auto layouts = *(u64 *)(This + 0x1F0);
	if (layouts == 0) return "choice_list_mdl_base_etc_sel";
	auto offset = *(i32 *)(This + 0x1D4);
	auto layout = *(AetLayoutData **)(layouts + ((index + offset) * 16));
	if (layout == 0) return "choice_list_mdl_base_etc_sel";

	LoadChoiceListPlayTxt (layout, module->attr, index);

	if ((module->attr & (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) == (ModuleAttr::FutureSound | ModuleAttr::ColorfulTone)) return "choice_list_mdl_base_etc_sel";
	else if (module->attr & ModuleAttr::FutureSound) return "choice_list_mdl_base_f_sel";
	else if (module->attr & ModuleAttr::ColorfulTone) return "choice_list_mdl_base_t_sel";
	else return "choice_list_mdl_base_etc_sel";
}

HOOK (void, SetModuleChoiceListPriority, 0x140691DC4);
HOOK (void, SetHairstyleChoiceListPriority, 0x140689375);
HOOK (void, SetModuleSprPriority, 0x140692C4A);
HOOK (void, SetHairstyleSprPriority, 0x140689F56);
HOOK (void, SetModuleSelectedPriority, 0x14069222C);
HOOK (void, SetHairstyleSelectedPriority, 0x140689889);

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

HOOK (u64, ModulePreviewInit, 0x1406962E0, u64 a1) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	return originalModulePreviewInit (a1);
}

HOOK (u64, HairstylePreviewInit, 0x14068BC90, u64 a1) {
	for (size_t i = 0; i < COUNTOFARR (choiceListPackId); i++)
		StopAet (&choiceListPackId[i]);
	return originalHairstylePreviewInit (a1);
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

i32 mdlPlateIndex[10]    = {0};
const char *characters[] = {"mdl_plate_mei", "mdl_plate_other", "mdl_plate_rand", "mdl_plate_mik", "mdl_plate_rin", "mdl_plate_len", "mdl_plate_luk", "mdl_plate_kai"};
i32 characterOffsets[]   = {3, 2, 1, 0, 7, 6, 5, 4};
i32 mdlIconIndex[8][6]   = {{0}};
bool inited              = false;
i32 oldChara             = 0;
HOOK (void, DrawMdlPlate, 0x140693830, u64 a1, i32 a2, u8 isIn) {
	if (!isIn) {
		for (i32 i = 0; i < 10; i++)
			StopAet (&mdlPlateIndex[i]);
		return;
	}

	if (a1 == 0) return;
	auto data = *(u64 *)(a1 + 0x08);
	if (data == 0) return;
	i32 selectedChara = *(i32 *)(data + 0x40);
	auto hashmap      = *(u64 *)(data + 0x958);
	auto hash         = *(u64 *)(a1 + 0xD8);
	if (hashmap == 0 || hash == 0) return;
	auto hashermap = (map<u64, AetLayerArgs *> *)(hashmap + 0x10);
	auto charaList = hashermap->find (hash);
	if (!charaList.has_value ()) return;
	AetComposition comp;
	GetComposition (&comp, (*charaList.value ())->id);

	char layerName[64];
	if (oldChara == 7 && selectedChara == 0) strcpy (layerName, "mdl_chara_list_up");
	else if (oldChara == 0 && selectedChara == 7) strcpy (layerName, "mdl_chara_list_down");
	else if (oldChara < selectedChara) strcpy (layerName, "mdl_chara_list_up");
	else if (oldChara > selectedChara) strcpy (layerName, "mdl_chara_list_down");
	else strcpy (layerName, "mdl_chara_list_loop");
	strcpy ((*(charaList.value ()))->layerName, layerName);
	(*(charaList.value ()))->play (&(*charaList.value ())->id);

	oldChara = selectedChara;

	if (strcmp (layerName, "mdl_chara_list_loop") != 0) {
		GetComposition (&comp, (*charaList.value ())->id);
		if (auto layout = comp.find (string ("p_chara_list00_c"))) {
			auto charaIndex = -selectedChara + 7;
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], 10, AetAction::IN_ONCE);
			args.position = layout.value ()->position;
			args.color.w  = layout.value ()->opacity;
			args.play (&mdlPlateIndex[0]);
		}
		if (auto layout = comp.find (string ("p_chara_list08_c"))) {
			auto charaIndex = selectedChara + 8;
			while (charaIndex > 7)
				charaIndex -= 8;
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], 10, AetAction::IN_ONCE);
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
			AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", characters[charaIndex], i == 3 ? 11 : 10, i == 3 ? AetAction::LOOP : AetAction::IN_ONCE);
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
	auto hashmap      = *(u64 *)(data + 0x958);
	auto hash         = *(u64 *)(a1 + 0xD8);
	if (hashmap == 0 || hash == 0) return;
	auto hashermap = (map<u64, AetLayerArgs *> *)(hashmap + 0x10);
	auto charaList = hashermap->find (hash);
	if (!charaList.has_value ()) return;

	AetComposition comp;
	GetComposition (&comp, (*charaList.value ())->id);

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

					AetLayerArgs args ("AET_NSWGAM_CUSTOM_MAIN", buf, 12, AetAction::NONE);
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
	auto data = *(u64 *)(a1 + 0x08);
	if (data == 0) return;
	i32 selectedChara = *(i32 *)(data + 0x40);
	auto hashmap      = *(u64 *)(data + 0x958);
	auto hash         = *(u64 *)(a1 + 0xD8);
	if (hashmap == 0 || hash == 0) return;
	auto hashermap = (map<u64, AetLayerArgs *> *)(hashmap + 0x10);
	auto charaList = hashermap->find (hash);
	if (!charaList.has_value ()) return;

	if (inited) {
		if (strcmp ((*(charaList.value ()))->layerName, "mdl_chara_list_down") == 0 || strcmp ((*(charaList.value ()))->layerName, "mdl_chara_list_up") == 0) {
			if (auto layer = aets->find ((*(charaList.value ()))->id)) {
				if (layer.value ()->currentFrame >= layer.value ()->loopEnd) {
					strcpy ((*(charaList.value ()))->layerName, "mdl_chara_list_loop");
					(*(charaList.value ()))->play (&(*charaList.value ())->id);
					StopAet (&mdlPlateIndex[0]);
					StopAet (&mdlPlateIndex[9]);
				}
			}
		}
	}

	AetComposition comp;
	GetComposition (&comp, (*charaList.value ())->id);

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

	INSTALL_HOOK (SetModuleChoiceListPriority);
	INSTALL_HOOK (SetHairstyleChoiceListPriority);
	INSTALL_HOOK (SetModuleSprPriority);
	INSTALL_HOOK (SetHairstyleSprPriority);
	INSTALL_HOOK (SetModuleSelectedPriority);
	INSTALL_HOOK (SetHairstyleSelectedPriority);
	INSTALL_HOOK (Memset);

	INSTALL_HOOK (DestroyModuleSelect);
	INSTALL_HOOK (DestroyHairstyleSelect);
	INSTALL_HOOK (ModulePreviewInit);
	INSTALL_HOOK (HairstylePreviewInit);

	taskAddition addition;
	addition.loop    = CustomizeSelLoop;
	addition.destroy = CustomizeSelDestroy;
	addTaskAddition ("CustomizeSel", addition);

	// Use the right font
	WRITE_MEMORY (0x140692A3D, i8, 0x10); // Modules
	WRITE_MEMORY (0x140689C48, i8, 0x10); // Hairstyles
	WRITE_MEMORY (0x14068DC1D, i8, 0x10); // Accessories

	WRITE_MEMORY (0x1406920B4, i32, 22); // Module name box priority
	WRITE_MEMORY (0x14068965F, i32, 22); // Hairstyle name box priority
	WRITE_MEMORY (0x140692D50, i32, 23); // Module name text priority
	WRITE_MEMORY (0x14068A05C, i32, 23); // Hairstyle name text priority
	WRITE_MEMORY (0x14069217B, i32, 22); // Module VP cost box priority
	WRITE_MEMORY (0x1406897C7, i32, 22); // Hairstyle VP cost box priority
	WRITE_MEMORY (0x1406930EE, i32, 23); // Module VP cost text priority
	WRITE_MEMORY (0x14068A457, i32, 23); // Hairstyle VP cost text priority
	WRITE_MEMORY (0x140692398, i32, 21); // Reccomended module priority
	WRITE_NOP (0x1406923A3, 4);

	WRITE_NOP (0x14069223D, 4);
	WRITE_NOP (0x140689899, 4);

	WRITE_MEMORY (0x140677FA9, i32, 25); // Choice_conf priority
	WRITE_MEMORY (0x140677E86, i32, 26); // Choice_conf button priority

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
}
} // namespace customize
