#include "diva.h"
#include "steam.h"

namespace leaderboard {

using namespace diva;
using namespace steam;

struct LeaderboardPv {
	i32 id;
	string name;
	i32 pack;
	i32 star;
	i32 unk_30;
	bool unk_34;
	bool unk_35;
	bool unk_36;
	bool unk_37;
	f32 unk_38;
	i32 unk_3C;
	i32 unk_40;
};

LeaderboardDownloadManager *leaderboardManager         = nullptr;
LeaderboardDownloadManager *personalLeaderboardManager = nullptr;

i32 base_id;
i32 cursor_id;
i32 my_id;
i32 my_none_id;
i32 load_id;
i32 entries_id[10];
i32 arrow_up_id;
i32 arrow_down_id;

u8 lastDiff;
i32 lastFilter;

u64 currentSelected;
u64 currentLocalSelected;
u64 offset;

void
RankingLoadBase (const char *prefix) {
	char layer_buf[64];

	sprintf (layer_buf, "%s_base", prefix);
	AetLayerArgs base ("AET_PS4_GALLERY_MAIN", layer_buf, 0x13, AetAction::IN_LOOP);
	base.play (&base_id);

	sprintf (layer_buf, "%s_my_base", prefix);
	AetLayerArgs my ("AET_PS4_GALLERY_MAIN", layer_buf, 0x13, AetAction::IN_LOOP);
	my.play (&my_id);

	AetLayerArgs load ("AET_PS4_GALLERY_MAIN", "hiscore_load", 0x15, AetAction::IN_LOOP);
	load.play (&load_id);

	AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
	up.play (&arrow_up_id);

	AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
	down.play (&arrow_down_id);

	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);
	for (int i = 0; i < 10; i++) {
		sprintf (buf, "p_%s_base%02d_c", prefix, i + 1);
		if (auto layout = comp.find (string (buf))) {
			sprintf (layer_buf, "%s_base_03", prefix);
			AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", layer_buf, 0x14, AetAction::NONE);
			entry.position = layout.value ()->position;
			entry.color.w  = layout.value ()->opacity;
			entry.play (&entries_id[i]);
		}
	}
}

void
RankingLoadedNone (const char *prefix) {
	char layer_buf[64];

	StopAet (&load_id);

	sprintf (layer_buf, "%s_my_none", prefix);
	AetLayerArgs me_none ("AET_PS4_GALLERY_MAIN", layer_buf, 0x15, AetAction::LOOP);
	me_none.play (&my_none_id);

	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);
	for (u64 i = 0; i < 10; i++) {
		sprintf (buf, "p_%s_base%02lld_c", prefix, i + 1);
		if (auto layout = comp.find (string (buf))) {
			sprintf (layer_buf, "%s_base_03", prefix);
			AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", layer_buf, 0x14, AetAction::NONE);
			entry.position = layout.value ()->position;
			entry.color.w  = layout.value ()->opacity;
			entry.play (&entries_id[i]);
		}
	}
}

template <typename T>
void
RankingLoaded (const char *prefix, std::vector<T> loaded) {
	char layer_buf[64];

	StopAet (&load_id);

	if (loaded.size () > 0) {
		AetLayerArgs cursor ("AET_PS4_GALLERY_MAIN", "hiscore_base_cursor", 0x14, AetAction::LOOP);
		cursor.play (&cursor_id);
	}

	AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
	up.play (&arrow_up_id);

	if (loaded.size () > 1) {
		AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::LOOP);
		down.play (&arrow_down_id);
	} else {
		AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
		down.play (&arrow_down_id);
	}

	if ((!personalLeaderboardManager->GetScores ().has_value () || personalLeaderboardManager->GetScores ().value ().size () == 0) &&
	    (!personalLeaderboardManager->GetAchievements ().has_value () || personalLeaderboardManager->GetAchievements ().value ().size () == 0)) {
		sprintf (layer_buf, "%s_my_none", prefix);
		AetLayerArgs me_none ("AET_PS4_GALLERY_MAIN", layer_buf, 0x15, AetAction::LOOP);
		me_none.play (&my_none_id);
	} else {
		StopAet (&my_none_id);
	}

	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);
	for (u64 i = 0; i < 10; i++) {
		sprintf (buf, "p_%s_base%02lld_c", prefix, i + 1);
		if (auto layout = comp.find (string (buf))) {
			sprintf (layer_buf, "%s_%s", prefix, loaded.size () <= i ? "base_03" : i % 2 == 0 ? "base_01" : "base_02");
			AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", layer_buf, 0x14, AetAction::NONE);
			entry.position = layout.value ()->position;
			entry.color.w  = layout.value ()->opacity;
			entry.play (&entries_id[i]);
		}
	}
}

void
RankingReload (const char *prefix) {
	AetLayerArgs load ("AET_PS4_GALLERY_MAIN", "hiscore_load", 0x15, AetAction::IN_LOOP);
	load.play (&load_id);
	StopAet (&cursor_id);

	currentSelected      = 0;
	currentLocalSelected = 0;
	offset               = 0;
}

template <typename T>
void
RankingScroll (const char *prefix, std::vector<T> loaded) {
	char layer_buf[64];

	void *inputState = diva::GetInputState (0);
	if (IsButtonDown (inputState, Button::UP) && currentSelected != 0) {
		currentSelected -= 1;

		if (currentLocalSelected == 0) {
			currentLocalSelected = 9;
			offset -= 10;

			sprintf (layer_buf, "%s_base_up", prefix);
			AetLayerArgs base ("AET_PS4_GALLERY_MAIN", layer_buf, 0x13, AetAction::IN_LOOP);
			base.play (&base_id);

			char buf[64];
			AetComposition comp;
			GetComposition (&comp, base_id);
			for (u64 i = 0; i < 10; i++) {
				sprintf (buf, "p_%s_base%02lld_c", prefix, i + 1);
				if (auto layout = comp.find (string (buf))) {
					sprintf (layer_buf, "%s_%s", prefix, loaded.size () <= i ? "base_03" : i % 2 == 0 ? "base_01" : "base_02");
					AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", layer_buf, 0x14, AetAction::NONE);
					entry.position = layout.value ()->position;
					entry.color.w  = layout.value ()->opacity;
					entry.play (&entries_id[i]);
				}
			}
		} else currentLocalSelected -= 1;

		if (currentSelected == 0) {
			AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
			up.play (&arrow_up_id);
		} else {
			AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::SPECIAL_LOOP);
			up.play (&arrow_up_id);
		}

		AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::LOOP);
		down.play (&arrow_down_id);
	} else if (IsButtonDown (inputState, Button::DOWN) && currentSelected != loaded.size () - 1) {
		currentSelected += 1;
		if (currentLocalSelected == 9) {
			currentLocalSelected = 0;
			offset += 10;

			sprintf (layer_buf, "%s_base_down", prefix);
			AetLayerArgs base ("AET_PS4_GALLERY_MAIN", layer_buf, 0x13, AetAction::IN_LOOP);
			base.play (&base_id);

			char buf[64];
			AetComposition comp;
			GetComposition (&comp, base_id);
			for (u64 i = 0; i < 10; i++) {
				sprintf (buf, "p_%s_base%02lld_c", prefix, i + 1);
				if (auto layout = comp.find (string (buf))) {
					sprintf (layer_buf, "%s_%s", prefix, loaded.size () <= i ? "base_03" : i % 2 == 0 ? "base_01" : "base_02");
					AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", layer_buf, 0x14, AetAction::NONE);
					entry.position = layout.value ()->position;
					entry.color.w  = layout.value ()->opacity;
					entry.play (&entries_id[i]);
				}
			}
		} else currentLocalSelected += 1;

		AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::LOOP);
		up.play (&arrow_up_id);

		if (currentSelected == loaded.size () - 1) {
			AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
			down.play (&arrow_down_id);
		} else {
			AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::SPECIAL_LOOP);
			down.play (&arrow_down_id);
		}
	}
}

template <typename T>
void
RankingDisplay (const char *prefix, T data, u64 i) {
	FontInfo font;
	GetSpriteFont (&font, 0xBE37, 38, 46);
	SetFontSize (&font, 26.0, 30.0);

	DrawParams params;
	params.font           = &font;
	params.layer          = 0x16;
	params.resolutionMode = RESOLUTION_MODE_FHD;
	params.colour[0]      = 0xFF;
	params.colour[1]      = 0xFF;

	params.colour[2] = 0xFF;
	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);

	sprintf (buf, "p_%s_grade%02lld_c", prefix, i + 1);
	if (auto layout = comp.find (string (buf))) {
		auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
		params.lineOrigin  = position;
		params.textCurrent = position;
		params.colour[3]   = layout.value ()->opacity * 0xFF;
		DrawTextFmt (&params, 0x28, "%d", data.rank);
	}

	sprintf (buf, "p_%s_rank%02lld_c", prefix, i + 1);
	if (auto layout = comp.find (string (buf))) {
		auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
		params.lineOrigin  = position;
		params.textCurrent = position;
		params.colour[3]   = layout.value ()->opacity * 0xFF;
		DrawTextFmt (&params, 0x28, "%02d", data.playerRank);
	}

	GetLangFont (&font, FontId::FNT_36_DIVA_36_38, true);
	SetFontSize (&font, 36.0, 38.0);
	params.colour[0] = 0x36;
	params.colour[1] = 0x46;
	params.colour[2] = 0x49;

	sprintf (buf, "p_%s_id%02lld_c", prefix, i + 1);
	if (auto layout = comp.find (string (buf))) {
		auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
		params.lineOrigin  = position;
		params.textCurrent = position;
		params.colour[3]   = layout.value ()->opacity * 0xFF;
		DrawTextFmt (&params, 0x28, data.playerName.c_str ());
	}
}

void
RankingStop () {
	currentSelected      = 0;
	currentLocalSelected = 0;
	offset               = 0;

	StopAet (&base_id);
	StopAet (&cursor_id);
	StopAet (&my_id);
	StopAet (&my_none_id);
	StopAet (&load_id);
	for (int i = 0; i < 10; i++)
		StopAet (&entries_id[i]);
	StopAet (&arrow_up_id);
	StopAet (&arrow_down_id);
}

bool
HiScoreInit (u64 task) {
	lastDiff   = 0;
	lastFilter = 0;

	return false;
}

bool
HiScoreLoop (u64 task) {
	i32 state     = *(i32 *)(task + 0x6C);
	i32 filter    = *(i32 *)(task + 0x74);
	u8 difficulty = *(u8 *)(task + 0x78);
	i32 pvIndex   = *(i32 *)(task + 0xCE8);
	auto pvs      = (vector<LeaderboardPv> *)(task + 0x1098);
	auto id       = pvs->at (pvIndex).value ()->id;

	if (state == 3 && leaderboardManager == nullptr) {
		*(i32 *)(task + 0x6C) = 2;
		leaderboardManager    = new LeaderboardDownloadManager ();
		leaderboardManager->DownloadScores (id, difficulty, filter, false);

		personalLeaderboardManager = new LeaderboardDownloadManager ();
		personalLeaderboardManager->DownloadScores (id, difficulty, 1, true);

		RankingLoadBase ("hiscore");

		lastDiff   = difficulty;
		lastFilter = filter;
	}

	if (state == 2) {
		leaderboardManager->Update ();
		auto scores = leaderboardManager->GetScores ();
		if (leaderboardManager->HasFailed ()) {
			*(i32 *)(task + 0x6C) = 3;
			RankingLoadedNone ("hiscore");
		} else if (scores.has_value ()) {
			*(i32 *)(task + 0x6C) = 3;
			RankingLoaded ("hiscore", scores.value ());
		}
	}

	if (state == 3 && leaderboardManager != nullptr) {
		leaderboardManager->Update ();
		if (difficulty != lastDiff || filter != lastFilter) {
			*(i32 *)(task + 0x6C) = 2;
			leaderboardManager->DownloadScores (id, difficulty, filter, false);
			if (difficulty != lastDiff) personalLeaderboardManager->DownloadScores (id, difficulty, 1, true);

			RankingReload ("hiscore");
		}

		lastDiff   = difficulty;
		lastFilter = filter;

		auto scores = leaderboardManager->GetScores ();
		if (scores.has_value ()) RankingScroll ("hiscore", scores.value ());
	}

	if (state == 1 && leaderboardManager != nullptr) {
		delete leaderboardManager;
		leaderboardManager = nullptr;

		delete leaderboardManager;
		personalLeaderboardManager = nullptr;

		RankingStop ();
	}

	return false;
}

bool
HiScoreDisplay (u64 task) {
	i32 state = *(i32 *)(task + 0x6C);
	if (state == 2 || state == 3) {
		FontInfo font;
		GetSpriteFont (&font, 0xBE37, 38, 46);
		SetFontSize (&font, 26.0, 30.0);

		DrawParams params;
		params.font           = &font;
		params.layer          = 0x16;
		params.resolutionMode = RESOLUTION_MODE_FHD;
		params.colour[0]      = 0xFF;
		params.colour[1]      = 0xFF;
		params.colour[2]      = 0xFF;

		char buf[64];
		AetComposition comp;
		GetComposition (&comp, base_id);

		sprintf (buf, "p_hiscore_base%02lld_c", currentLocalSelected + 1);
		auto layer  = aets->find (cursor_id);
		auto layout = comp.find (string (buf));
		if (layout.has_value () && layer.has_value ()) {
			layer.value ()->position = layout.value ()->position;
			layer.value ()->color.w  = layout.value ()->opacity;
		}

		for (u64 i = 0; i < 10; i++) {
			sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
			auto layer  = aets->find (entries_id[i]);
			auto layout = comp.find (string (buf));
			if (!layout.has_value () || !layer.has_value ()) continue;
			layer.value ()->position = layout.value ()->position;
			layer.value ()->color.w  = layout.value ()->opacity;

			if (leaderboardManager == nullptr) continue;
			auto scores = leaderboardManager->GetScores ();
			if (!scores.has_value () || scores.value ().size () <= i + offset) continue;
			auto score = scores.value ().at (i + offset);

			sprintf (buf, "p_hiscore_score%02lld_rt", i + 1);
			if (auto layout = comp.find (string (buf))) {
				auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 2.0, layout.value ()->position.y + layout.value ()->height / 3.0);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%06d", score.score);
			}

			RankingDisplay ("hiscore", score, i);
		}

		if (personalLeaderboardManager != nullptr && personalLeaderboardManager->GetScores ().has_value () && personalLeaderboardManager->GetScores ().value ().size () == 1) {
			StopAet (&my_none_id);

			GetComposition (&comp, my_id);

			if (auto layout = comp.find (string ("p_hiscore_my_score01_rt"))) {
				auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 2.0, layout.value ()->position.y + layout.value ()->height / 3.0);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%06d", personalLeaderboardManager->GetScores ().value ().front ().score);
			}

			if (auto layout = comp.find (string ("p_hiscore_my_grade01_c"))) {
				auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%d", personalLeaderboardManager->GetScores ().value ().front ().rank);
			}
		}
	}

	return false;
}

bool waiting;

bool
AchieveInit (u64 task) {
	lastDiff   = 0;
	lastFilter = 0;
	waiting    = true;

	bool isCT                  = *(u8 *)(task + 0x104) == 2;
	leaderboardManager         = new LeaderboardDownloadManager ();
	personalLeaderboardManager = new LeaderboardDownloadManager ();

	if (isCT) {
		leaderboardManager->DownloadCTAchievements (0, 0, false);
		personalLeaderboardManager->DownloadCTAchievements (0, 1, true);
	} else {
		leaderboardManager->DownloadFSAchievements (0, 0, false);
		personalLeaderboardManager->DownloadFSAchievements (0, 1, true);
	}

	RankingLoadBase ("achieve");
	return false;
}

const i32 songCounts[2][3] = {{128, 128, 55}, {110, 110, 45}};

bool
AchieveLoop (u64 task) {
	bool isCT     = *(u8 *)(task + 0x104) == 2;
	u8 filter     = *(u8 *)(task + 0xD8);
	u8 difficulty = *(u8 *)(task + 0xDC);

	if (difficulty != lastDiff || filter != lastFilter) {
		waiting = true;
		if (isCT) {
			leaderboardManager->DownloadCTAchievements (difficulty, filter, false);
			if (difficulty != lastDiff) personalLeaderboardManager->DownloadCTAchievements (difficulty, 1, true);
		} else {
			leaderboardManager->DownloadFSAchievements (difficulty, filter, false);
			if (difficulty != lastDiff) personalLeaderboardManager->DownloadFSAchievements (difficulty, 1, true);
		}

		RankingReload ("achieve");
	}

	leaderboardManager->Update ();
	if (waiting) {
		auto achievements = leaderboardManager->GetAchievements ();
		if (leaderboardManager->HasFailed ()) {
			waiting = false;
			RankingLoadedNone ("achieve");
		} else if (achievements.has_value ()) {
			waiting = false;
			RankingLoaded ("achieve", achievements.value ());
		}
	}

	auto achievements = leaderboardManager->GetAchievements ();
	if (achievements.has_value ()) RankingScroll ("achieve", achievements.value ());

	lastDiff   = difficulty;
	lastFilter = filter;

	return false;
}

bool
AchieveDisplay (u64 task) {
	bool isCT     = *(u8 *)(task + 0x104) == 2;
	u8 difficulty = *(u8 *)(task + 0xDC);

	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);

	sprintf (buf, "p_achieve_base%02lld_c", currentLocalSelected + 1);
	auto layer  = aets->find (cursor_id);
	auto layout = comp.find (string (buf));
	if (layout.has_value () && layer.has_value ()) {
		layer.value ()->position = layout.value ()->position;
		layer.value ()->color.w  = layout.value ()->opacity;
	}

	FontInfo font;
	GetSpriteFont (&font, 0xBE37, 38, 46);
	SetFontSize (&font, 26.0, 30.0);

	DrawParams params;
	params.font           = &font;
	params.layer          = 0x16;
	params.resolutionMode = RESOLUTION_MODE_FHD;
	params.colour[0]      = 0xFF;
	params.colour[1]      = 0xFF;
	params.colour[2]      = 0xFF;

	for (u64 i = 0; i < 10; i++) {
		sprintf (buf, "p_achieve_base%02lld_c", i + 1);
		auto layer  = aets->find (entries_id[i]);
		auto layout = comp.find (string (buf));
		if (!layout.has_value () || !layer.has_value ()) continue;
		layer.value ()->position = layout.value ()->position;
		layer.value ()->color.w  = layout.value ()->opacity;

		if (leaderboardManager == nullptr) continue;
		auto achievements = leaderboardManager->GetAchievements ();
		if (!achievements.has_value () || achievements.value ().size () <= i + offset) continue;
		auto achievement = achievements.value ().at (i + offset);

		f32 clearPercentage = achievement.combinedPercentage / songCounts[isCT][difficulty];
		f32 left            = floor (clearPercentage);
		f32 right           = clearPercentage - left;

		sprintf (buf, "p_achieve_achieve_num01_%02lld_rt", i + 1);
		if (auto layout = comp.find (string (buf))) {
			auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 1.5, layout.value ()->position.y + layout.value ()->height / 3.0);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%04.0f", right * 10000.0);
		}

		sprintf (buf, "p_achieve_achieve_num02_%02lld_rt", i + 1);
		if (auto layout = comp.find (string (buf))) {
			auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width, layout.value ()->position.y + layout.value ()->height / 3.0);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%03.0f", left);
		}

		RankingDisplay ("achieve", achievement, i);
	}

	auto personalAchivement = personalLeaderboardManager->GetAchievements ();
	if (personalAchivement.has_value () && personalAchivement.value ().size () == 1) {
		StopAet (&my_none_id);

		f32 clearPercentage = personalAchivement.value ().front ().combinedPercentage / songCounts[isCT][difficulty];
		f32 left            = floor (clearPercentage);
		f32 right           = clearPercentage - left;

		GetComposition (&comp, my_id);
		if (auto layout = comp.find (string ("p_achieve_my_achieve_num01_rt"))) {
			auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 1.5, layout.value ()->position.y + layout.value ()->height / 3.0);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%04.0f", right * 10000.0);
		}

		if (auto layout = comp.find (string ("p_achieve_my_achieve_num02_rt"))) {
			auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width, layout.value ()->position.y + layout.value ()->height / 3.0);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%03.0f", left);
		}

		if (auto layout = comp.find (string ("p_achieve_my_grade01_c"))) {
			auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%d", personalAchivement.value ().front ().rank);
		}
	}

	return false;
}

bool
AchieveDestroy (u64 task) {
	delete leaderboardManager;
	leaderboardManager = nullptr;

	delete leaderboardManager;
	personalLeaderboardManager = nullptr;

	RankingStop ();

	return false;
}

bool
SurvivalInit (u64 task) {
	lastFilter = 0;

	return false;
}

bool
SurvivalLoop (u64 task) {
	i32 state          = *(i32 *)(task + 0x180);
	i32 selectedCourse = *(i32 *)(task + 0x188);
	i32 filter         = *(i32 *)(task + 0x288);

	if (state == 1 && leaderboardManager == nullptr) {
		leaderboardManager = new LeaderboardDownloadManager ();
		leaderboardManager->DownloadSurvivalScores (selectedCourse, filter, false);

		personalLeaderboardManager = new LeaderboardDownloadManager ();
		personalLeaderboardManager->DownloadSurvivalScores (selectedCourse, 1, true);

		RankingLoadBase ("survival");

		lastFilter = filter;
		waiting    = true;
	}

	if (leaderboardManager != nullptr) {
		if (filter != lastFilter) {
			lastFilter = filter;
			waiting    = true;
			leaderboardManager->DownloadSurvivalScores (selectedCourse, filter, false);

			RankingReload ("survival");
		}

		leaderboardManager->Update ();
		if (waiting) {
			auto scores = leaderboardManager->GetScores ();
			if (leaderboardManager->HasFailed ()) {
				waiting = false;
				RankingLoadedNone ("survival");
			} else if (scores.has_value ()) {
				waiting = false;
				RankingLoaded ("survival", scores.value ());
			}
		}

		auto scores = leaderboardManager->GetScores ();
		if (scores.has_value ()) RankingScroll ("survival", scores.value ());
	}

	if (state == 2 && leaderboardManager != nullptr) {
		RankingStop ();
		delete leaderboardManager;
		leaderboardManager = nullptr;

		delete leaderboardManager;
		personalLeaderboardManager = nullptr;
	}

	return false;
}

bool
SurvivalDisplay (u64 task) {
	char buf[64];
	AetComposition comp;
	GetComposition (&comp, base_id);

	FontInfo font;
	GetSpriteFont (&font, 0xBE37, 38, 46);
	SetFontSize (&font, 26.0, 30.0);

	DrawParams params;
	params.font           = &font;
	params.layer          = 0x16;
	params.resolutionMode = RESOLUTION_MODE_FHD;
	params.colour[0]      = 0xFF;
	params.colour[1]      = 0xFF;
	params.colour[2]      = 0xFF;

	sprintf (buf, "p_survival_base%02lld_c", currentLocalSelected + 1);
	auto layer  = aets->find (cursor_id);
	auto layout = comp.find (string (buf));
	if (layout.has_value () && layer.has_value ()) {
		layer.value ()->position = layout.value ()->position;
		layer.value ()->color.w  = layout.value ()->opacity;
	}

	for (u64 i = 0; i < 10; i++) {
		sprintf (buf, "p_survival_base%02lld_c", i + 1);
		auto layer  = aets->find (entries_id[i]);
		auto layout = comp.find (string (buf));
		if (!layout.has_value () || !layer.has_value ()) continue;
		layer.value ()->position = layout.value ()->position;
		layer.value ()->color.w  = layout.value ()->opacity;

		if (leaderboardManager == nullptr) continue;
		auto scores = leaderboardManager->GetScores ();
		if (!scores.has_value () || scores.value ().size () <= i + offset) continue;
		auto score = scores.value ().at (i + offset);

		sprintf (buf, "p_survival_score%02lld_rt", i + 1);
		if (auto layout = comp.find (string (buf))) {
			auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 2.4, layout.value ()->position.y + layout.value ()->height / 3.0);
			params.lineOrigin  = position;
			params.textCurrent = position;
			params.colour[3]   = layout.value ()->opacity * 0xFF;
			DrawTextFmt (&params, 0x28, "%07d", score.score);
		}

		RankingDisplay ("survival", score, i);
	}

	if (personalLeaderboardManager != nullptr) {
		auto personalScore = personalLeaderboardManager->GetScores ();
		if (personalScore.has_value () && personalScore.value ().size () == 1) {
			StopAet (&my_none_id);

			GetComposition (&comp, my_id);

			if (auto layout = comp.find (string ("p_survival_my_score01_rt"))) {
				auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 2.4, layout.value ()->position.y + layout.value ()->height / 3.0);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%07d", personalLeaderboardManager->GetScores ().value ().front ().score);
			}

			if (auto layout = comp.find (string ("p_survival_my_grade01_c"))) {
				auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%d", personalLeaderboardManager->GetScores ().value ().front ().rank);
			}
		}
	}

	return false;
}

void
init () {
	taskAddition hiscore;
	hiscore.init    = HiScoreInit;
	hiscore.loop    = HiScoreLoop;
	hiscore.display = HiScoreDisplay;
	addTaskAddition ("CS_RANKING_HI_SCORE", hiscore);

	taskAddition achieve;
	achieve.init    = AchieveInit;
	achieve.loop    = AchieveLoop;
	achieve.display = AchieveDisplay;
	achieve.destroy = AchieveDestroy;
	addTaskAddition ("CS_RANKING_ACHIEVE_RATE", achieve);

	taskAddition survival;
	survival.init    = SurvivalInit;
	survival.loop    = SurvivalLoop;
	survival.display = SurvivalDisplay;
	addTaskAddition ("CS_RANKING_SURVIVAL", survival);
}

} // namespace leaderboard
