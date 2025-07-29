#include "diva.h"

namespace pvSel {
typedef enum Style : i32 {
	STYLE_FT   = 0,
	STYLE_MM   = 1,
	STYLE_NONE = 2,
} Style;

using namespace diva;

bool hasClicked = false;
i32 pvId        = 0;
char buttonName[16];
char selectorImgName[64];
i32 selectorId    = 0;
i32 selectorImgId = 0;
i32 keyHelpId     = 0;
InputType lastInputType;
Vec4 touchArea;
Vec3 keyHelpLoc;
Vec3 txtLoc;
bool playing = true;

bool optSelectorInited = false;
i32 optSelectorId      = 0;
Vec4 topButton;
Vec4 topButtonLeft;
Vec4 topButtonRight;
Vec4 middleButton;
Vec4 middleButtonLeft;
Vec4 middleButtonRight;
Vec4 bottomButton;
Vec4 bottomButtonLeft;
Vec4 bottomButtonRight;
Vec4 startButton;
Vec4 startButtonLeft;
Vec4 startButtonRight;

i32 lastCover = 0;

void *nswgamPVSelTask = malloc (0x27540);

void
updateStyleAets (Style newStyle) {
	int i;
	switch (newStyle) {
	case STYLE_FT: i = 2; break;
	case STYLE_MM: i = 1; break;
	case STYLE_NONE: i = 3; break;
	}
	sprintf (selectorImgName, "nswgam_songselector_visual_settings_%02d.pic", i);

	AetLayerArgs selectorImgData ("AET_PS4_MENU_MAIN", selectorImgName, 0x12, AetAction::NONE);
	selectorImgData.setPosition (txtLoc);
	selectorImgData.play (&selectorImgId);
}

void
updateButtonPrompt (InputType input) {
	sprintf (buttonName, "visual_key_%02d", (u8)input);

	AetLayerArgs keyHelpData ("AET_PS4_MENU_MAIN", buttonName, 0x13, AetAction::NONE);
	keyHelpData.setPosition (keyHelpLoc);
	keyHelpData.play (&keyHelpId);
}

void
initStyle (Style style, InputType input) {
	AetLayerArgs selectorData ("AET_PS4_MENU_MAIN", "visual_settings", 0x12, AetAction::NONE);
	selectorData.play (&selectorId);

	AetComposition compositionData;
	GetComposition (&compositionData, selectorId);

	if (auto buttonPlaceholderData = compositionData.find (string ("key_help_lv_tab_01"))) keyHelpLoc = buttonPlaceholderData.value ()->position;
	if (auto textPlaceholderData = compositionData.find (string ("visual_settings_txt"))) txtLoc = textPlaceholderData.value ()->position;
	if (auto buttonTouchAreaData = compositionData.find (string ("p_visual_settings_touch"))) touchArea = getPlaceholderRect (**buttonTouchAreaData);

	updateStyleAets (style);
	updateButtonPrompt (input);
}

Style
getStyle (i32 currentStyle, bool isMovie) {
	if (isMovie) return STYLE_NONE;
	else return (Style)currentStyle;
}

void
initOptionsSelectTouch () {
	AetLayerArgs optSelectorData ("AET_PS4_MENU_MAIN", "conf_set_touch", 0, AetAction::NONE);
	optSelectorData.play (&optSelectorId);
	AetComposition compositionData;
	GetComposition (&compositionData, optSelectorId);

	if (auto layer = compositionData.find ("p_conf_set_base01_touch_c")) topButton = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_l01_touch_c")) topButtonLeft = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_r01_touch_c")) topButtonRight = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_base02_touch_c")) middleButton = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_l02_touch_c")) middleButtonLeft = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_r02_touch_c")) middleButtonRight = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_base03_touch_c")) bottomButton = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_l03_touch_c")) bottomButtonLeft = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_set_arrow_r03_touch_c")) bottomButtonRight = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_start_btn_touch_c")) startButton = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_start_arrow_l_touch_c")) startButtonLeft = getPlaceholderRect (*layer.value ());
	if (auto layer = compositionData.find ("p_conf_start_arrow_r_touch_c")) startButtonRight = getPlaceholderRect (*layer.value ());

	StopAet (&optSelectorId);
}

FUNCTION_PTR (void, PlayButtonOut, 0x14020c010, u64, u8);
FUNCTION_PTR (void, UpdateButtons, 0x14020b3b0, u64, u8, u8, u8, u8);
FUNCTION_PTR (void, UpdateSubMenu, 0x140209460, u64, i32, u64, i32, i32, u8, u8, u64, u64, u8, u8);
void
updateSelectedButton (u64 This, i32 selectedButton) {
	*(i32 *)(This + 0x78) = selectedButton;
	PlaySoundEffect ("se_ft_sys_select_01", 1.0);
	UpdateButtons (This + 0x78, 0, 0, 0, 0);
}

void
optionsSelectTouch (u64 This) {
	if (!optSelectorInited) {
		initOptionsSelectTouch ();
		optSelectorInited = true;
	}

	void *inputState = diva::GetInputState (0);
	Vec2 clickedPos  = getClickedPos (inputState);
	if (hasClicked) return;
	if (clickedPos.x <= 0) {
		hasClicked = false;
		return;
	}
	hasClicked = true;

	i32 selectedButton   = *(i32 *)(This + 0x78);
	i32 subMenu          = *(i32 *)(This + 0x9C);
	bool extraVocals     = *(bool *)(This + 0xB4);
	i32 extraVocalsCount = 0;
	if (extraVocals) extraVocalsCount = (*(u64 *)(This + 0x480) - *(u64 *)(This + 0x478)) / 0x68;
	bool extraStage = *(bool *)(This + 0xB5);
	bool success    = *(bool *)(This + 0xB6);

	bool topButtonEnabled    = subMenu != 1 || success;
	bool middleButtonEnabled = subMenu != 1 || extraVocals;
	bool bottomButtonEnabled = (subMenu == 1 && extraStage) || (subMenu != 1 && (extraVocals || extraStage));

	if (startButton.contains (clickedPos) && selectedButton != 0) updateSelectedButton (This, 0);
	else if (topButton.contains (clickedPos) && selectedButton != 1 && topButtonEnabled) updateSelectedButton (This, 1);
	else if (middleButton.contains (clickedPos) && selectedButton != 2 && middleButtonEnabled) updateSelectedButton (This, 2);
	else if (bottomButton.contains (clickedPos) && selectedButton != 3 && bottomButtonEnabled) updateSelectedButton (This, 3);

	if (subMenu == 0 || subMenu == 2) {
		if (selectedButton == 1) {
			if (subMenu == 0) {
				if (topButtonLeft.contains (clickedPos) || topButtonRight.contains (clickedPos)) {
					*(bool *)(This + 0xA0) = !*(bool *)(This + 0xA0);
					PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
					UpdateButtons (This + 0x78, 0, 0, 0, 0);
				}
			} else if (subMenu == 2) {
				float maxTime = *(float *)(This + 0x36A3C) - 30.0;
				if (topButtonLeft.contains (clickedPos)) {
					i32 startTime = *(i32 *)(This + 0xB0) - 10;
					if (startTime < 0) startTime = std::floor ((float)maxTime / 10) * 10;
					*(i32 *)(This + +0xB0) = startTime;
					PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
				} else if (topButtonRight.contains (clickedPos)) {
					i32 startTime = *(i32 *)(This + 0xB0) + 10;
					if (startTime > maxTime) startTime = 0;
					*(i32 *)(This + +0xB0) = startTime;
					PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
				}
			}
		} else if (selectedButton == 2) {
			if (middleButtonLeft.contains (clickedPos)) {
				i32 *modifier = (i32 *)(This + 0xA4);
				if (*modifier == 0) *modifier = 3;
				else *modifier = *modifier - 1;
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			} else if (middleButtonRight.contains (clickedPos)) {
				i32 *modifier = (i32 *)(This + 0xA4);
				if (*modifier == 3) *modifier = 0;
				else *modifier = *modifier + 1;
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			}
		} else if (selectedButton == 3) {
			if (extraVocals) {
				if (bottomButtonLeft.contains (clickedPos)) {
					*(i32 *)(This + 0xA8) = *(i32 *)(This + 0xA8) - 1;
					if (*(i32 *)(This + 0xA8) < 0) *(i32 *)(This + 0xA8) = extraVocalsCount - 1;
					PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
				} else if (bottomButtonRight.contains (clickedPos)) {
					*(i32 *)(This + 0xA8) = *(i32 *)(This + 0xA8) + 1;
					if (*(i32 *)(This + 0xA8) >= extraVocalsCount) *(i32 *)(This + 0xA8) = 0;
					PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
				}
			} else if (bottomButtonLeft.contains (clickedPos) || bottomButtonRight.contains (clickedPos)) {
				*(bool *)(This + 0xAC) = !*(bool *)(This + 0xAC);
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			}
		}
	} else if (subMenu == 1) {
		if (selectedButton == 1) {
			if (topButtonLeft.contains (clickedPos) || topButtonRight.contains (clickedPos)) {
				*(bool *)(This + 0xAD) = !*(bool *)(This + 0xAD);
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			}
		} else if (selectedButton == 2) {
			if (bottomButtonLeft.contains (clickedPos)) {
				*(i32 *)(This + 0xA8) = *(i32 *)(This + 0xA8) - 1;
				if (*(i32 *)(This + 0xA8) < 0) *(i32 *)(This + 0xA8) = extraVocalsCount - 1;
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			} else if (bottomButtonRight.contains (clickedPos)) {
				*(i32 *)(This + 0xA8) = *(i32 *)(This + 0xA8) + 1;
				if (*(i32 *)(This + 0xA8) >= extraVocalsCount) *(i32 *)(This + 0xA8) = 0;
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			}
		} else if (selectedButton == 3) {
			if (bottomButtonLeft.contains (clickedPos) || bottomButtonRight.contains (clickedPos)) {
				*(bool *)(This + 0xAC) = !*(bool *)(This + 0xAC);
				PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			}
		}
	}

	if (startButtonLeft.contains (clickedPos)) {
		if (subMenu == 0) subMenu = 2;
		else subMenu = subMenu - 1;

		*(i32 *)(This + 0x78) = 0;
		UpdateButtons (This + 0x78, 1, 0, 0, 0);
		PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);

		UpdateSubMenu (This + 0x78, subMenu, This + 0xB8, *(i32 *)(This + 0xA8), *(i32 *)(This + 0xB0), *(u8 *)(This + 0xB5), *(u8 *)(This + 0xB6), This + 0xB8, This + 0x490, 0, *(u8 *)(This + 0xB7));
		UpdateButtons (This + 0x78, 1, 0, 0, 0);
	} else if (startButtonRight.contains (clickedPos)) {
		if (subMenu == 2) subMenu = 0;
		else subMenu = subMenu + 1;

		*(i32 *)(This + 0x78) = 0;
		PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);

		UpdateSubMenu (This + 0x78, subMenu, This + 0xB8, *(i32 *)(This + 0xA8), *(i32 *)(This + 0xB0), *(u8 *)(This + 0xB5), *(u8 *)(This + 0xB6), This + 0xB8, This + 0x490, 0, *(u8 *)(This + 0xB7));
		UpdateButtons (This + 0x78, 0, 1, 0, 0);
	} else if (startButton.contains (clickedPos) && selectedButton == 0) {
		PlaySoundEffect ("se_ft_music_selector_enter_01", 1.0);
		*(u8 *)(This + 0x99) = 1;
		*(u8 *)(This + 0x9B) = 1;
		PlayButtonOut (This + 0x78, 1);
	}
}

FUNCTION_PTR (void, PlayMusic, 0x140201320, void *, const char *file, i32 pvId, f32 sabiStartTime, f32 sabiPlayTime, f32 totalLength);
FUNCTION_PTR (void, FUN1401EAD50, 0x1401EAD50, u64, i32, void *);
FUNCTION_PTR (void, UpdatePerformerDisplay, 0x14020FBE0, u64, u64 performerCount, u64, bool);

bool
PVSelLoop (u64 This) {
	// Touch
	if (*(i32 *)(This + 0x68) == 8) optionsSelectTouch (This);

	// Allow swapping of visual style on song select
	// Disable on playlists
	if (*(u8 *)(0x14CC10480) || !playing) return false;

	auto entry   = getPvDbEntry (*(i32 *)(This + 0x36A30));
	auto diff    = *(i32 *)(This + 0x7830);
	bool isMovie = false;
	if (entry) isMovie = isMovieOnly (*entry);

	InputType input  = getInputType ();
	void *inputState = diva::GetInputState (0);
	u64 pvLoadData   = GetPvLoadData ();
	i32 style        = *(i32 *)(pvLoadData + 0x1D08);

	if (input == InputType::UNKNOWN) input = InputType::PLAYSTATION;

	if (style == -1) {
		style = GetCurrentStyle ();
		if (style == -1) style = STYLE_FT;
		initStyle (getStyle (style, isMovie), input);
		lastInputType = input;
	}

	if (input != lastInputType) {
		lastInputType = input;
		updateButtonPrompt (input);
	}

	if (IsButtonTapped (inputState, Button::L3) && !isMovie) {
		PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
		style = !style;
		updateStyleAets (getStyle (style, isMovie));
	} else if (pvId != *(i32 *)(This + 0x36A30)) {
		style = GetCurrentStyle ();
		updateStyleAets (getStyle (style, isMovie));
		pvId = *(i32 *)(This + 0x36A30);
	}

	Vec2 clickedPos = getClickedPos (inputState);
	if (clickedPos.x > 0 && !hasClicked) {
		hasClicked = true;
		if (touchArea.contains (clickedPos) && !isMovie) {
			style = !style;
			PlaySoundEffect ("se_ft_music_selector_select_01", 1.0);
			updateStyleAets (getStyle (style, isMovie));
		}
	} else if (clickedPos.x == 0) hasClicked = false;

	*(i32 *)(pvLoadData + 0x1D08)           = style;
	*(u8 *)((u64)nswgamPVSelTask + 0x27538) = (u8)style; // Fix Future Tone Customization

	if (*(i32 *)(This + 0xA8) != lastCover) {
		auto covers = (vector<CoverSong> *)(This + 0x478);
		if (auto cover = covers->at (*(i32 *)(This + 0xA8))) {
			*(i32 *)(This + 0x36A08) = 1;
			memcpy ((void *)(This + 0x36A10), (void *)&cover.value ()->fileName, sizeof (string));
			PlayMusic ((void *)(This + 0x36A08), cover.value ()->fileName.c_str (), *(i32 *)(This + 0x36A30), *(f32 *)(This + 0x36A34), *(f32 *)(This + 0x36A38), *(f32 *)(This + 0x36A3C));
		}
	}

	lastCover = *(i32 *)(This + 0xA8);

	if (IsButtonDown (inputState, Button::R3) && entry.has_value () && *(i32 *)((u64)GetSaveData () + 0x169410) != 3) {
		*(i32 *)((u64)GetSaveData () + 0x169410) = 3;
		auto score                               = FindScore (GetSaveData (), entry.value ()->id);
		for (u64 i = 0; i < entry.value ()->performers.length (); i++) {
			auto performer = entry.value ()->performers.at (i).value ();
			if (performer->fixed) continue;
			if (performer->pseudo_same_id != -1) performer = entry.value ()->performers.at (performer->pseudo_same_id).value ();

			auto cos = performer->pv_costume[diff];
			if (cos == -1) cos = 0;

			ModuleData *module = nullptr;
			for (auto it = modules->begin (); it != modules->end (); it++) {
				if (it->chara == performer->chara && it->cos == cos) {
					module = it;
					break;
				}
			}
			if (module == nullptr || !CheckModuleUnlocked (FindModule (GetSaveData (), module->id))) continue;

			auto save_data             = (SaveDataModule *)((u64)score + 0x10E0 + (i * sizeof (SaveDataModule)));
			save_data->moduleId        = module->id;
			save_data->accessory_head  = performer->item[0];
			save_data->accessory_face  = performer->item[1];
			save_data->accessory_chest = performer->item[2];
			save_data->accessory_back  = performer->item[3];
			save_data->hair            = -1;
		}

		FUN1401EAD50 (0x141750890, *(i32 *)(This + 0x373DC), nullptr);
		auto performerCount = std::count_if (entry.value ()->performers.begin (), entry.value ()->performers.end (), [] (auto &it) { return !it.fixed; });
		UpdatePerformerDisplay (This + 0x55E8, performerCount, This + 0x36a68, true);

		PlaySoundEffect ("se_ft_sys_enter_01", 1.0);
	}

	return false;
}

void
hide () {
	playing = false;
	StopAet (&keyHelpId);
	StopAet (&selectorImgId);
	StopAet (&selectorId);
}

void
unhide () {
	playing = true;
}

bool
PvSelDisplay (u64 This) {
	// Disable on playlist
	if (*(u8 *)(0x14CC10480) || selectorId == 0 || !playing) return false;

	AetComposition compositionData;
	GetComposition (&compositionData, selectorId);

	if (auto buttonPlaceholderData = compositionData.find (string ("key_help_lv_tab_01"))) keyHelpLoc = buttonPlaceholderData.value ()->position;
	if (auto textPlaceholderData = compositionData.find (string ("visual_settings_txt"))) txtLoc = textPlaceholderData.value ()->position;
	if (auto buttonTouchAreaData = compositionData.find (string ("p_visual_settings_touch"))) touchArea = getPlaceholderRect (**buttonTouchAreaData);

	AetLayerArgs keyHelpData ("AET_PS4_MENU_MAIN", buttonName, 0x13, AetAction::NONE);
	keyHelpData.setPosition (keyHelpLoc);
	keyHelpData.play (&keyHelpId);

	AetLayerArgs selectorImgData ("AET_PS4_MENU_MAIN", selectorImgName, 0x12, AetAction::NONE);
	selectorImgData.setPosition (txtLoc);
	selectorImgData.play (&selectorImgId);

	return false;
}

// Hacky fix for song count in bottom left without needing to update the aet
HOOK (void, GetListNumAllData, 0x14020E2F0, u64 a1, u32 index, Vec3 *position, Vec3 *scale, u32 *colour) {
	if (index < 4) {
		originalGetListNumAllData (a1, index, position, scale, colour);
	} else {
		auto comp  = (AetComposition *)(a1 + 0x1130);
		auto num02 = comp->find (string ("p_list_all_num02_c"));
		auto num03 = comp->find (string ("p_list_all_num03_c"));
		auto diff  = num02.value ()->position.x - num03.value ()->position.x;
		if (position != 0) *position = Vec3 (num03.value ()->position.x - (diff * (index - 3)), num03.value ()->position.y, num03.value ()->position.z);
		if (scale != 0) *scale = Vec3 (num03.value ()->matrix.x.x, num03.value ()->matrix.y.y, num03.value ()->matrix.z.z);
		if (colour != 0) *colour = ((u8)(num03.value ()->opacity * 255.0) << 18) | 0xFFFFFF;
	}
}

const i32 gameReleaseOrder[] = {
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  27,  28,  29,  30,  31,  32,  101, 201, 202, 203, 401, 402,
    403, 404, 405, 407, 408, 409, 410, 411, 412, 204, 205, 206, 207, 102, 208, 209, 210, 211, 38,  41,  43,  58,  59,  48,  49,  54,  56,  62,  413, 414, 415, 416, 417, 55,  64,  66,  212,
    37,  44,  50,  51,  65,  418, 419, 420, 421, 422, 39,  40,  47,  53,  60,  213, 423, 424, 425, 426, 427, 42,  57,  61,  63,  45,  46,  52,  428, 429, 82,  86,  215, 90,  94,  430, 431,
    87,  88,  93,  96,  89,  95,  97,  81,  83,  84,  79,  91,  92,  85,  214, 220, 227, 228, 218, 225, 226, 231, 103, 216, 232, 104, 221, 222, 224, 233, 234, 219, 236, 223, 235, 600, 602,
    616, 626, 628, 603, 619, 622, 613, 638, 239, 240, 615, 238, 433, 601, 629, 607, 611, 620, 432, 608, 610, 435, 614, 639, 434, 617, 618, 624, 242, 612, 625, 243, 630, 642, 436, 437, 438,
    621, 439, 440, 640, 641, 441, 631, 730, 244, 241, 442, 710, 443, 627, 637, 246, 604, 605, 250, 609, 623, 247, 724, 727, 722, 723, 732, 248, 739, 740, 251, 729, 736, 725, 726, 734, 731,
    737, 728, 733, 738, 249, 257, 259, 260, 255, 261, 253, 832, 262, 265, 254, 263, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281,
};

struct PvSelData {
	PvSpriteId *pv;
	bool unk08;
	bool isRandom;
	bool unk0A;
	bool unk0B;
	INSERT_PADDING (0x1C);
	bool unk28;
	u8 unk29;
	bool unk2A;
	f32 unk2C;
	string name;
};

FUNCTION_PTR (void, UpdateSongListPlaceholders, 0x140211D00, u64 a1, u64 a2);
FUNCTION_PTR (void, DisplaySongList, 0x1402066C0, u64 a1, bool, bool, bool);

const char *SortLayerName = "sort_set_mod";
i32 SortIndex             = 0;

char modsPrefix[MAX_PATH];
i32 ModdedIndex = 0;
std::map<std::string, std::set<i32>> ModSongs;
std::vector<std::string> ModSongIndicies;
vector<PvSelData *> FilteredSongs;
PvSelData randomData;

HOOK (void, CreateSortedPVList, 0x140206C30, u64 a1) {
	if (*(i32 *)(a1 + 0x373E0) != 4) return originalCreateSortedPVList (a1);

	auto songs = *(vector<PvSelData> **)(a1 + 0x37308);
	auto diff  = (i32 *)(a1 + 0x373F8);
	auto extra = (bool *)(a1 + 0x373FC);

	FilteredSongs.clear ();
	while (FilteredSongs.length () == 0) {
		if (ModdedIndex < (i32)ModSongs.size ()) {
			for (auto it = songs->begin (); it != songs->end (); it++) {
				if (!ModSongs[ModSongIndicies[ModdedIndex]].contains (it->pv->pvData->id) || !it->pv->pvData->HasDifficulty (*diff, *extra)) continue;
				auto score           = FindScore (GetSaveData (), it->pv->pvData->id);
				auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
				if (!IsScoreDifficultyUnlocked (scoreDifficulty)) continue;
				FilteredSongs.push_back (it);
			}
		} else if (ModdedIndex == (i32)ModSongs.size ()) {
			for (auto it = songs->begin (); it != songs->end (); it++) {
				if (!it->pv->pvData->HasDifficulty (*diff, *extra)) continue;
				auto score           = FindScore (GetSaveData (), it->pv->pvData->id);
				auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
				if (!IsScoreDifficultyUnlocked (scoreDifficulty)) continue;
				FilteredSongs.push_back (it);
			}
		} else if (ModdedIndex == (i32)ModSongs.size () + 1) {
			for (auto it = songs->begin (); it != songs->end (); it++) {
				if (!it->pv->pvData->HasDifficulty (*diff, *extra)) continue;
				auto score           = FindScore (GetSaveData (), it->pv->pvData->id);
				auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
				if (!IsScoreDifficultyUnlocked (scoreDifficulty) || !*(bool *)((u64)score + 0x119F)) continue;
				FilteredSongs.push_back (it);
			}
		}

		if (FilteredSongs.length () == 0) {
			ModdedIndex += *(i32 *)(a1 + 0x373F4);
			if (ModdedIndex >= (i32)ModSongs.size () + 2) ModdedIndex = 0;
			else if (ModdedIndex < 0) ModdedIndex = ModSongs.size () + 1;
		}
	}

	if (ModdedIndex == 0)
		std::sort (std::execution::par, FilteredSongs.begin (), FilteredSongs.end (), [] (auto a, auto b) {
			u64 a_index = 0;
			u64 b_index = 0;
			for (u64 i = 0; i < 254; i++) {
				if (gameReleaseOrder[i] == a->pv->pvData->id) a_index = i;
				if (gameReleaseOrder[i] == b->pv->pvData->id) b_index = i;
				if (a_index && b_index) break;
			}

			if (a_index == 0 || b_index == 0) return a->pv->pvData->id < b->pv->pvData->id;
			else return a_index < b_index;
		});
	else std::sort (std::execution::par, FilteredSongs.begin (), FilteredSongs.end (), [] (auto a, auto b) { return a->pv->pvData->id < b->pv->pvData->id; });

	FilteredSongs.push_back (&randomData);

	*(vector<PvSelData *> **)(a1 + 0x36FD8) = &FilteredSongs;

	*(i32 *)(a1 + 0x55F4)  = FilteredSongs.length ();
	*(i32 *)(a1 + 0x37378) = FilteredSongs.length () - 1;

	auto id   = *(i32 *)(a1 + 0x36A30);
	i32 index = 0;
	for (u64 i = 0; i < FilteredSongs.length (); i++) {
		auto pv = **FilteredSongs.at (i);
		if (pv->pv != nullptr && pv->pv->pvData->id == id) {
			index = i;
			break;
		}
	}
	*(i32 *)(a1 + 0x55E8)  = index;
	*(i32 *)(a1 + 0x55F8)  = index;
	*(i32 *)(a1 + 0x36FC8) = index;

	UpdateSongListPlaceholders (a1 + 0x55E8, a1 + 0x36EE8);

	// For new classics
	*(i32 *)(a1 + 0x373E0) = 0;
}

HOOK (void, ChangeSort, 0x140207650, u64 a1) {
	auto sort = (i32 *)(a1 + 0x373E0);
	if (*sort == 4) {
		*sort                                   = -1;
		*(vector<PvSelData *> **)(a1 + 0x36FD8) = nullptr;
		return originalChangeSort (a1);
	} else if (*sort != 3) return originalChangeSort (a1);

	*sort                   = 4;
	*(i32 **)(a1 + 0x373D0) = &SortIndex;
	*(i32 *)(a1 + 0x373CC)  = 1;

	auto diff                             = (i32 *)(a1 + 0x373F8);
	auto extra                            = (bool *)(a1 + 0x373FC);
	*(vector<PvSelData> **)(a1 + 0x37308) = (vector<PvSelData> *)(a1 + (*diff * 2 + 0x24AB + *extra) * 0x18);

	FilteredSongs.clear ();
	while (FilteredSongs.length () <= 1) {
		whereCreateSortedPVList (a1);
		*(i32 *)(a1 + 0x373E0) = 4;
		if (FilteredSongs.length () <= 1) {
			ModdedIndex += *(i32 *)(a1 + 0x373F4);
			if (ModdedIndex >= (i32)ModSongs.size () + 2) ModdedIndex = 0;
			else if (ModdedIndex < 0) ModdedIndex = ModSongs.size () + 1;
		}
	}

	auto songs = *(vector<PvSelData> **)(a1 + 0x37308);

	i32 totalCount = std::count_if (std::execution::par, songs->begin (), songs->end (), [&] (auto &it) {
		if (!it.pv->pvData->HasDifficulty (*diff, *extra)) return false;
		auto score           = FindScore (GetSaveData (), it.pv->pvData->id);
		auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
		return IsScoreDifficultyUnlocked (scoreDifficulty);
	});

	*(i32 *)(a1 + 0x3737C) = totalCount;

	DisplaySongList (a1, true, false, true);
}

HOOK (void, ChangeFilter, 0x1402078D0, u64 a1, i32 direction) {
	if (*(u32 *)(a1 + 0x373E0) != 4 || IsSurvival ()) return originalChangeFilter (a1, direction);

	*(i32 *)(a1 + 0x373F4) = direction;

	FilteredSongs.clear ();
	while (FilteredSongs.length () <= 1) {
		ModdedIndex += direction;
		if (ModdedIndex >= (i32)ModSongs.size () + 2) ModdedIndex = 0;
		else if (ModdedIndex < 0) ModdedIndex = ModSongs.size () + 1;

		whereCreateSortedPVList (a1);
		*(i32 *)(a1 + 0x373E0) = 4;
	}

	auto songs = *(vector<PvSelData> **)(a1 + 0x37308);
	auto diff  = (i32 *)(a1 + 0x373F8);
	auto extra = (bool *)(a1 + 0x373FC);

	i32 totalCount = std::count_if (std::execution::par, songs->begin (), songs->end (), [&] (auto &it) {
		if (!it.pv->pvData->HasDifficulty (*diff, *extra)) return false;
		auto score           = FindScore (GetSaveData (), it.pv->pvData->id);
		auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
		return IsScoreDifficultyUnlocked (scoreDifficulty);
	});

	*(i32 *)(a1 + 0x3737C) = totalCount;

	DisplaySongList (a1, true, true, true);
}

HOOK (void, ChangeDiff, 0x140207550, u64 a1, i32 direction) {
	if (*(u32 *)(a1 + 0x373E0) != 4 || IsSurvival ()) return originalChangeDiff (a1, direction);
	if (direction == 0) return;

	auto diff  = (i32 *)(a1 + 0x373F8);
	auto extra = (bool *)(a1 + 0x373FC);

	if (direction > 0) {
		if (*diff == 3) {
			if (*extra == true) {
				*diff  = 0;
				*extra = false;
			} else {
				*extra = true;
			}
		} else {
			*diff += direction;
		}
	} else if (direction < 0) {
		if (*diff == 3) {
			if (*extra == true) *extra = false;
			else *diff += direction;
		} else if (*diff == 0) {
			*diff  = 3;
			*extra = true;
		} else {
			*diff += direction;
		}
	}

	*(vector<PvSelData> **)(a1 + 0x37308) = (vector<PvSelData> *)(a1 + (*diff * 2 + 0x24AB + *extra) * 0x18);
	auto songs                            = *(vector<PvSelData> **)(a1 + 0x37308);

	FilteredSongs.clear ();
	while (FilteredSongs.length () <= 1) {
		whereCreateSortedPVList (a1);
		*(i32 *)(a1 + 0x373E0) = 4;
		if (FilteredSongs.length () <= 1) {
			if (*diff == 3) {
				if (*extra == true) *extra = false;
				else *diff += direction;
			} else if (*diff == 0) {
				*diff  = 3;
				*extra = true;
			} else {
				*diff += direction;
			}
		}
	}

	i32 totalCount = std::count_if (std::execution::par, songs->begin (), songs->end (), [&] (auto &it) {
		if (!it.pv->pvData->HasDifficulty (*diff, *extra)) return false;
		auto score           = FindScore (GetSaveData (), it.pv->pvData->id);
		auto scoreDifficulty = GetScoreDifficulty (score, 0, *diff, *extra);
		return IsScoreDifficultyUnlocked (scoreDifficulty);
	});

	*(i32 *)(a1 + 0x3737C) = totalCount;

	DisplaySongList (a1, false, false, true);
}

HOOK (void, UpdatePvListData, 0x140207AB0, u64 a1) {
	if (*(u32 *)(a1 + 0x373E0) != 4 || IsSurvival ()) return originalUpdatePvListData (a1);
	if (ModdedIndex != (i32)ModSongs.size () + 1) return;

	whereCreateSortedPVList (a1);
	*(i32 *)(a1 + 0x373E0) = 4;

	auto id   = *(i32 *)(a1 + 0x36A30);
	i32 index = *(i32 *)(a1 + 0x55E8);
	if (index != 0) index -= 1;
	for (u64 i = 0; i < FilteredSongs.length (); i++) {
		auto pv = **FilteredSongs.at (i);
		if (pv->pv != nullptr && pv->pv->pvData->id == id) {
			index = i;
			break;
		}
	}
	*(i32 *)(a1 + 0x55E8)  = index;
	*(i32 *)(a1 + 0x55F8)  = index;
	*(i32 *)(a1 + 0x36FC8) = index;

	DisplaySongList (a1, false, false, true);
}

bool
PvSelInit (u64 This) {
	if (*(i32 *)(This + 0x373E0) == 4) *(i32 *)(This + 0x373E0) = 0;
	unhide ();
	u64 pvLoadData = GetPvLoadData ();
	if (pvLoadData) *(i32 *)(pvLoadData + 0x1D08) = -1;
	lastCover = 0;

	return false;
}

bool
PvSelDestroy (u64 This) {
	*(vector<PvSelData *> **)(This + 0x36FD8) = nullptr;
	hide ();
	return false;
}

HOOK (bool, PvDbRead, 0x1404BB290, u64 task) {
	auto res = originalPvDbRead (task);

	if (*(i32 *)(task + 0x68) == 0 && ModSongs.size () == 0) {
		for (auto it = pvs->begin (); it != pvs->end (); it++) {
			auto pv = *it;
			if (pv->id == 700 || pv->id == 701 || pv->id == 999) continue;

			char buf[MAX_PATH] = {'\0'};
			for (auto difficulty = 0; difficulty != 5; difficulty++) {
				for (auto edition = pv->difficulties[difficulty].begin (); edition != pv->difficulties[difficulty].end (); edition++) {
					if (edition->scriptFile.length > 0) {
						strcpy_s (buf, edition->scriptFile.c_str ());
						break;
					}
				}
				if (buf[0] != '\0') break;
			}

			if (buf[0] == '\0') continue;

			string path (buf);
			if (!ResolveFilePath (&path, &path)) continue;

			u64 offset = 0;
			while (path.c_str ()[offset] == '.')
				offset += 2;

			u64 length                         = strstr (path.c_str () + offset, buf) - (path.c_str () + offset);
			path.c_str ()[offset + length - 1] = '\0';

			bool fromGame = false;
			for (u64 i = 0; i < 254; i++) {
				if (gameReleaseOrder[i] == pv->id) {
					fromGame = true;
					break;
				}
			}

			if (strstr (path.c_str () + offset, modsPrefix) && !fromGame) {
				offset += strlen (modsPrefix) + 1;
				auto key = std::string (path.c_str () + offset);
				if (ModSongs.contains (key)) ModSongs[key].insert (pv->id);
				else ModSongs[key] = {pv->id};
			} else {
				auto key = std::string ("\0\0");
				if (ModSongs.contains (key)) ModSongs[key].insert (pv->id);
				else ModSongs[key] = {pv->id};
			}
		}

		std::set<i32> miscSongs = {};
		for (auto it = ModSongs.begin (); it != ModSongs.end ();) {
			if (it->second.size () <= 3) {
				for (auto &song : it->second)
					miscSongs.insert (song);
				it = ModSongs.erase (it);
			} else {
				it++;
			}
		}
		if (miscSongs.size () > 0) ModSongs[std::string ("\xFF\xFF")] = miscSongs;

		std::map<std::string, std::string> nameReplacements;
		char buf[MAX_PATH];
		for (auto it = ModSongs.begin (); it != ModSongs.end (); it++) {
			sprintf (buf, "%s/%s/config.toml", modsPrefix, it->first.c_str ());
			FILE *fp = fopen (buf, "r");
			if (fp == nullptr) continue;
			auto config = toml_parse_file (fp, nullptr, 0);
			fclose (fp);
			if (config == nullptr) continue;

			auto data = toml_string_in (config, "name");
			if (data.ok) nameReplacements[it->first] = std::string (data.u.s);

			toml_free (config);
		}

		for (auto it = nameReplacements.begin (); it != nameReplacements.end (); it++) {
			auto elem   = ModSongs.extract (it->first);
			elem.key () = it->second;
			ModSongs.insert (std::move (elem));
		}

		for (auto &mod : ModSongs)
			ModSongIndicies.push_back (mod.first);

		memset ((void *)&randomData, 0, sizeof (PvSelData));
		randomData.isRandom = true;

		ModdedIndex = ModSongs.size ();
	}
	return res;
}

void
init () {
	auto file   = fopen ("../../config.toml", "r");
	auto config = toml_parse_file (file, nullptr, 0);
	fclose (file);
	if (config) {
		auto data = toml_string_in (config, "mods");
		if (data.ok) strcpy (modsPrefix, data.u.s);
		else strcpy (modsPrefix, "mods");
		toml_free (config);
	} else {
		strcpy (modsPrefix, "mods");
	}

	taskAddition addition;
	addition.init    = PvSelInit;
	addition.loop    = PVSelLoop;
	addition.destroy = PvSelDestroy;
	addition.display = PvSelDisplay;
	addTaskAddition ("PVsel", addition);

	WRITE_MEMORY (0x14CC5EF18, void *, nswgamPVSelTask);
	WRITE_MEMORY (0x140BE9488, char *, SortLayerName);
	INSTALL_HOOK (GetListNumAllData);
	INSTALL_HOOK (ChangeSort);
	INSTALL_HOOK (ChangeFilter);
	INSTALL_HOOK (ChangeDiff);
	INSTALL_HOOK (UpdatePvListData);
	INSTALL_HOOK (CreateSortedPVList);
	INSTALL_HOOK (PvDbRead);
}
} // namespace pvSel
