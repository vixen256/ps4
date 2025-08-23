#include "SigScan.h"
#include "diva.h"
#include "menus/menus.h"

i32 theme;

using namespace diva;

bool wantsToSettings = false;
HOOK (void, ChangeSubGameState, 0x1527E49E0, State state, SubState subState) {
	auto cmnMenu = (Task *)(0x14114C370);
	if (cmnMenu->state != TaskState::RUNNING) {
		cmnMenu->request = TaskRequest::RUN;

		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x6C))) layer.value ()->color.w = 1.0;
		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x70))) layer.value ()->color.w = 1.0;
	}
	if (state == State::MENU_SWITCH) {
		state = State::CS_MENU;
	} else if (subState == SubState::CS_OPTION_MENU) {
		state           = State::MENU_SWITCH;
		subState        = SubState::OPTION_MENU_SWITCH;
		wantsToSettings = true;

		cmnMenu->request = TaskRequest::HIDE;
		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x6C))) layer.value ()->color.w = 0.0;
		if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x70))) layer.value ()->color.w = 0.0;
	} else if (subState == SubState::MENU_SWITCH) {
		if (wantsToSettings) {
			subState        = SubState::OPTION_MENU_SWITCH;
			wantsToSettings = false;

			cmnMenu->request = TaskRequest::HIDE;
			if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x6C))) layer.value ()->color.w = 0.0;
			if (auto layer = aets->find (*(i32 *)((u64)cmnMenu + 0x70))) layer.value ()->color.w = 0.0;
		} else {
			state    = State::CS_MENU;
			subState = SubState::CS_MENU;
		}
	}

	return originalChangeSubGameState (state, subState);
}

HOOK (i32 *, GetFtTheme, 0x1401D6540) { return &theme; }

std::set<std::string> themeStrings = {"option_sub_menu_eachsong",
                                      "option_sub_menu_allsong",
                                      "timing",
                                      "option_sub_menu_vibration",
                                      "option_sub_menu_bgm_volume",
                                      "option_sub_menu_button_volume",
                                      "option_sub_menu_se_volume",
                                      "option_sub_menu_back",
                                      "gam_btn_resume",
                                      "gam_btn_timing",
                                      "gam_btn_retry",
                                      "gam_btn_option",
                                      "gam_btn_back",
                                      "gam_btn_back_playlist",
                                      "nswgam_cmnbg_bg",
                                      "nswgam_adv_bg",
                                      "press_a_button",
                                      "footer_01",
                                      "option_top_menu loop",
                                      "fotter01",
                                      "fotter02",
                                      "fotter03",
                                      "fotter04",
                                      "fotter05",
                                      "fotter08",
                                      "fotter09",
                                      "setting_menu_bg_arcade_base_in",
                                      "setting_menu_bg_arcade_base_up",
                                      "setting_menu_bg_arcade_base_down",
                                      "bg02",
                                      "footer_02",
                                      "footer_03",
                                      "nswgam_tshirtsedit_colorselector_bg",
                                      "nswgam_tshirtsedit_keyhelp_bg",
                                      "cmn_win_help",
                                      "btn_close",
                                      "savedata_warning_dialog",
                                      "cmn_win_m",
                                      "cmn_menu_yes",
                                      "cmn_menu_no"};

std::set<std::string> themeStringsDx = {
    "press_a_button",    "nswgam_adv_bg",  "setting_menu_bg_arcade_base_in", "setting_menu_bg_arcade_base_up", "setting_menu_bg_arcade_base_down", "option_top_menu loop", "stat_base_ft",
    "stat_base_down_ft", "stat_base_up_ft"};

void
HandleLayer (AetLayer *layer) {
	if (layer->itemType == AetItemType::AET_ITEM_TYPE_AUDIO) layer->flags &= ~0x8000;
	else if (layer->itemType == AetItemType::AET_ITEM_TYPE_COMPOSITION)
		for (u32 i = 0; i != layer->item.comp->layersCount; i++)
			HandleLayer (&layer->item.comp->layers[i]);
}

HOOK (i32 *, PlayAetLayerH, 0x1402CA220, diva::AetLayerArgs *args, i32 id) {
	if (id != 0) {
		// This is *supposed to* prevent AET SEs from playing twice
		// By giving them a flag when played and removing that flag when the layer is replayed
		// But some layers are played once per frame so this dosent do anything for those
		// So some users will still experience duplicate SEs
		// The real solution to this is to remove them from the AETs and make them played from code
		auto aet = aets->find (id);
		if (aet.has_value ()) {
			auto comp = aet.value ()->comp;
			if (comp)
				for (u32 i = 0; i != comp->layersCount; i++)
					HandleLayer (&comp->layers[i]);
		}
	}

	if (args == 0 || args->layerName == 0 || (u64)args->layerName == (u64)(i64)-1) return originalPlayAetLayerH (args, id);

	if (theme == 3 && themeStringsDx.find (std::string (args->layerName)) != themeStringsDx.end ()) {
		if (strcmp (args->layerName, "stat_base_ft") == 0) args->layerName = "stat_base_dx";
		else if (strcmp (args->layerName, "stat_base_up_ft") == 0) args->layerName = "stat_base_up_dx";
		else if (strcmp (args->layerName, "stat_base_down_ft") == 0) args->layerName = "stat_base_down_dx";
		else args->layerName = diva::appendThemeDx (args->layerName);
	} else if (themeStrings.find (std::string (args->layerName)) != themeStrings.end ()) {
		if (strcmp (args->layerName, "gam_btn_retry") == 0 && diva::IsSurvival ()) {
			args->StartMarker = diva::string ("st_sp_02");
			args->EndMarker   = diva::string ("ed_sp_02");
		}
		args->layerName = diva::appendTheme (args->layerName);
	}

	return originalPlayAetLayerH (args, id);
}

HOOK (void, LoadAetFrameH, 0x1402CA590, void *data, i32 aetSceneId, const char *layerName, AetAction action, i32 layer, char *a6, float frame) {
	if (layerName == 0) return originalLoadAetFrameH (data, aetSceneId, layerName, action, layer, a6, frame);

	if (theme == 3) {
		for (auto str : themeStringsDx) {
			if (strcmp (str.c_str (), layerName) == 0) {
				const char *theme = diva::appendThemeDx (layerName);
				return originalLoadAetFrameH (data, aetSceneId, theme, action, layer, a6, frame);
			}
		}
	}

	for (auto str : themeStrings) {
		if (strcmp (str.c_str (), layerName) == 0) {
			const char *theme = diva::appendTheme (layerName);
			return originalLoadAetFrameH (data, aetSceneId, theme, action, layer, a6, frame);
		}
	}

	return originalLoadAetFrameH (data, aetSceneId, layerName, action, layer, a6, frame);
}

HOOK (void, OpenNpCommerce, 0x1401DEA30) { ShellExecuteA (NULL, "open", "https://store.steampowered.com/app/1761390", NULL, NULL, SW_SHOWNORMAL); }
HOOK (void, OpenCredits, 0x1401DEA00) { ShellExecuteA (NULL, "open", "https://divamodarchive.com/post/25", NULL, NULL, SW_SHOWNORMAL); }

extern "C" {

FUNCTION_PTR (float, GetLayerFrame, 0x1402CA120, i32 id, char *layer_name);
FUNCTION_PTR (diva::string *, AppendLayerSuffix, 0x14022D070, void *a1, diva::string *base_layer_name);
HOOK (void, CmnMenuTouchCheck, 0x14022C590);

FUNCTION_PTR (void, CtrlLayerReturn, 0x140290BA1);
FUNCTION_PTR (void, CtrlLayerContinue, 0x14029028C);
HOOK (void, CtrlLayer, 0x140290280);

HOOK (void, GetBgmIndexCsMenu, 0x1401B27F8);
HOOK (void, GetBgmIndexCsGalleryIn, 0x1401AE540);
HOOK (void, GetBgmIndexCsGalleryLoop, 0x1401AE5D0);
HOOK (void, GetBgmIndexCsGalleryOut, 0x1401AE660);
HOOK (void, GetBgmIndexCsResult1, 0x140235BE1);
HOOK (void, GetBgmIndexCsResult2, 0x140237B1F);
HOOK (void, GetBgmIndexCsResult3, 0x140237DFC);
HOOK (void, GetBgmIndexCsResult4, 0x140237730);

__declspec (dllexport) void
init () {
	freopen ("CONOUT$", "w", stdout);
	bool screenshot_loading_screen = true;
	auto file                      = fopen ("config.toml", "r");
	auto config                    = toml_parse_file (file, NULL, 0);
	fclose (file);
	if (!config) {
		theme = 0;
	} else {
		auto data = toml_int_in (config, "theme");
		if (!data.ok) theme = 0;
		else theme = data.u.i;

		data = toml_bool_in (config, "screenshot_loading_screen");
		if (!data.ok) screenshot_loading_screen = true;
		else screenshot_loading_screen = data.u.b;
	}

	INSTALL_HOOK (ChangeSubGameState);
	INSTALL_HOOK (GetFtTheme);
	INSTALL_HOOK (PlayAetLayerH);
	INSTALL_HOOK (LoadAetFrameH);
	INSTALL_HOOK (OpenNpCommerce);
	INSTALL_HOOK (OpenCredits);
	INSTALL_HOOK (CmnMenuTouchCheck);
	INSTALL_HOOK (CtrlLayer);
	INSTALL_HOOK (GetBgmIndexCsMenu);
	INSTALL_HOOK (GetBgmIndexCsGalleryIn);
	INSTALL_HOOK (GetBgmIndexCsGalleryLoop);
	INSTALL_HOOK (GetBgmIndexCsGalleryOut);
	INSTALL_HOOK (GetBgmIndexCsResult1);
	INSTALL_HOOK (GetBgmIndexCsResult2);
	INSTALL_HOOK (GetBgmIndexCsResult3);
	INSTALL_HOOK (GetBgmIndexCsResult4);

	// Turn on FT mode 1.00 Samyuu, 1.03 BroGamer
	WRITE_MEMORY (0x1414AB9E3, u8, 1);

	// Stop returning to ADV from main menu
	WRITE_NOP (0x1401B2ADA, 36);

	// Fix SFX select priority
	WRITE_MEMORY (0x140699000, u8, 0x0A);

	// Fix song select sort options
	WRITE_MEMORY (0x140BE9520, u64, 0x140C85CB0, 0x140C85CE8);

	// Reduce number of loading screens to 3
	WRITE_MEMORY (0x1406542B4, u8, 0x02);

	// Properly load rights_bg02
	WRITE_NULL (0x15E99F0A8, 1);

	// Use Left/Right on FX select
	WRITE_MEMORY (0x14069997F, Button, Button::LEFT);
	WRITE_MEMORY (0x1406999A6, Button, Button::RIGHT);

	// Load survival_attention from aet_nswgam_game in the pause menu
	WRITE_MEMORY (0x140657012, i32, 0x51C);

	WRITE_MEMORY (0x14066451D, AetAction, AetAction::IN_LOOP);
	WRITE_MEMORY (0x140654505, AetAction, AetAction::IN_LOOP);
	WRITE_MEMORY (0x1401A9CF8, AetAction, AetAction::IN_LOOP);

	// Autosave messages
	WRITE_MEMORY (0x140663883, AetAction, AetAction::IN_LOOP);
	WRITE_MEMORY (0x140666383, AetAction, AetAction::IN_LOOP);
	WRITE_MEMORY (0x140661E13, AetAction, AetAction::IN_LOOP);
	WRITE_MEMORY (0x140661813, AetAction, AetAction::IN_LOOP);

	// Swap order of BGM layers
	WRITE_MEMORY (0x140BE9080, u64, 0x140C7FB58);
	WRITE_MEMORY (0x140BE90A8, u64, 0x140C7FB58);
	WRITE_MEMORY (0x140BE90C8, u64, 0x140C7FB58);
	WRITE_MEMORY (0x140BE9090, u64, 0x140C7FB50);
	WRITE_MEMORY (0x140BE90B8, u64, 0x140C7FB50);
	WRITE_MEMORY (0x140BE90D8, u64, 0x140C7FB50);
	WRITE_MEMORY (0x140BE9C50, u64, 0x140C879D0);
	WRITE_MEMORY (0x140BE9C60, u64, 0x140C879E8);

	// Fix songs with weird difficulties
	WRITE_MEMORY (0x140BE92C0, u64, 0x140C86940, 0x140C86940);

	diva::init ();
	exitMenu::init ();
	pvSel::init ();
	gallery::init ();
	options::init ();
	pause::init ();
	customize::init ();
	result::init ();
	decoration::init ();
	pvWatch::init ();
	pvGame::init (screenshot_loading_screen);
	genericDialog::init ();
	commonUi::init ();
	commonMenu::init ();
	leaderboard::init ();
}

SIG_SCAN_STRING (sigPvDbSwitch, "pv_db_switch.txt");
SIG_SCAN_STRING (sigMenuTxt1, "menu_txt_01");
SIG_SCAN_STRING (sigMenuTxt2, "menu_txt_02");
SIG_SCAN_STRING (sigMenuTxt3, "menu_txt_03");
SIG_SCAN_STRING (sigMenuTxt4, "menu_txt_04");
SIG_SCAN_STRING (sigMenuTxt5, "menu_txt_05");
SIG_SCAN_STRING (sigMenuTxtBase, "menu_txt_base");
SIG_SCAN_STRING (sigCmnWinHelpG, "cmn_win_help_g_inout");
SIG_SCAN_STRING (sigCmnWinHelpP, "cmn_win_help_p_inout");
SIG_SCAN_STRING (sigCmnWinHelpR, "cmn_win_help_r_inout");
SIG_SCAN_STRING (sigCmnWinHelpY, "cmn_win_help_y_inout");
SIG_SCAN_STRING (sigCmnWinG, "cmn_win_g");
SIG_SCAN_STRING (sigCmnWinP, "cmn_win_p");

__declspec (dllexport) void
preInit () {
	WRITE_NULL (sigPvDbSwitch (), 1);
	WRITE_NULL (sigMenuTxt1 (), 1);
	WRITE_NULL (sigMenuTxt2 (), 1);
	WRITE_NULL (sigMenuTxt3 (), 1);
	WRITE_NULL (sigMenuTxt4 (), 1);
	WRITE_NULL (sigMenuTxt5 (), 1);
	WRITE_NULL (sigMenuTxtBase (), 1);

	WRITE_MEMORY (sigCmnWinHelpG (), char, "cmn_win_help");
	WRITE_MEMORY (sigCmnWinHelpP (), char, "cmn_win_help");
	WRITE_MEMORY (sigCmnWinHelpR (), char, "cmn_win_help");
	WRITE_MEMORY (sigCmnWinHelpY (), char, "cmn_win_help");

	WRITE_MEMORY (sigCmnWinG (), char, "cmn_win_m");
	WRITE_MEMORY (sigCmnWinP (), char, "cmn_win_m");
}

__declspec (dllexport) void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	pvGame::D3DInit (SwapChain, Device, DeviceContext);
}
}
