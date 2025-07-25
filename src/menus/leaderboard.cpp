#include "diva.h"

using namespace diva;

namespace leaderboard {

struct Score {
	i32 rank;
	i32 score;
	u64 playerId;
	std::string playerName;
	i32 playerRank;
};

// Kinda annoying interface but steam wants it for callbacks
class LeaderboardManager {
public:
	bool gettingPersonalScore = false;

private:
	i32 state = -1;
	ELeaderboardDataRequest filter;
	CCallResult<LeaderboardManager, LeaderboardFindResult_t> m_LeaderboardFindResult;
	CCallResult<LeaderboardManager, LeaderboardScoresDownloaded_t> m_LeaderboardDownloadResult;
	SteamLeaderboard_t leaderboard;
	std::vector<Score> scores;

	void FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure) {
		if (failure || !res->m_bLeaderboardFound) {
			this->state = 0;
			return;
		}

		this->leaderboard = res->m_hSteamLeaderboard;
		i32 start         = 1;
		i32 end           = 100;
		if (this->filter == k_ELeaderboardDataRequestGlobalAroundUser) {
			if (gettingPersonalScore) {
				start = 0;
				end   = 0;
			} else {
				start = -4;
				end   = 5;
			}
		}
		auto callback = SteamUserStats ()->DownloadLeaderboardEntries (this->leaderboard, this->filter, start, end);
		this->m_LeaderboardDownloadResult.Set (callback, this, &LeaderboardManager::DownloadLeaderboardCallback);
		this->state = -1;
	}

	void DownloadLeaderboardCallback (LeaderboardScoresDownloaded_t *res, bool failure) {
		if (failure) {
			this->state = 1;
			return;
		}

		this->scores.clear ();
		for (int i = 0; i < res->m_cEntryCount; i++) {
			LeaderboardEntry_t entry;
			i32 details[2];
			SteamUserStats ()->GetDownloadedLeaderboardEntry (res->m_hSteamLeaderboardEntries, i, &entry, details, 2);

			Score score;
			score.rank       = entry.m_nGlobalRank;
			score.score      = entry.m_nScore;
			score.playerId   = entry.m_steamIDUser.ConvertToUint64 ();
			score.playerRank = details[0];

			if (SteamFriends ()->RequestUserInformation (entry.m_steamIDUser, true) == false) score.playerName = std::string (SteamFriends ()->GetFriendPersonaName (entry.m_steamIDUser));
			else score.playerName = std::string ("");

			this->scores.push_back (score);
		}

		u64 i = 0;
		for (; i < this->scores.size (); i++)
			if (this->scores[i].playerName.size () == 0) break;

		if (i == this->scores.size ()) this->state = 2;
		else this->state = -1;
	}

	STEAM_CALLBACK (LeaderboardManager, OnPersonaStateChange, PersonaStateChange_t);

public:
	void Update () { SteamAPI_RunCallbacks (); }

	void LoadLeaderboard (i32 id, i32 diff, i32 filter) {
		this->filter = (ELeaderboardDataRequest)filter;
		char buf[64];
		sprintf (buf, "SCORE_%05dD%cA%cMA", id, diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
		auto callback = SteamUserStats ()->FindLeaderboard (buf);
		this->m_LeaderboardFindResult.Set (callback, this, &LeaderboardManager::FindLeaderboardCallback);

		this->state = -1;
	}

	bool HasFailed () {
		if (this->state == 0 || this->state == 1) return true;
		else return false;
	}

	std::optional<std::vector<Score>> GetScores () {
		if (this->state == 2) return this->scores;
		else return {};
	}
};

void
LeaderboardManager::OnPersonaStateChange (PersonaStateChange_t *ret) {
	if (this->scores.size () == 0 || (ret->m_nChangeFlags & k_EPersonaChangeName) != k_EPersonaChangeName) return;
	for (u64 i = 0; i < this->scores.size (); i++) {
		if (this->scores[i].playerId != ret->m_ulSteamID) continue;
		this->scores[i].playerName = SteamFriends ()->GetFriendPersonaName (ret->m_ulSteamID);
		break;
	}

	u64 i = 0;
	for (; i < scores.size (); i++)
		if (this->scores[i].playerName.size () == 0) break;

	if (i == this->scores.size ()) this->state = 2;
	else this->state = -1;
}

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

LeaderboardManager *leaderboardManager         = nullptr;
LeaderboardManager *personalLeaderboardManager = nullptr;

i32 hiscore_base;
i32 hiscore_cursor;
i32 my_hiscore;
i32 my_hiscore_none;
i32 hiscore_load;
i32 hiscore_entries[10];
i32 arrow_up;
i32 arrow_down;

u8 lastDiff;
i32 lastFilter;

i32 currentSelected;
i32 currentLocalSelected;
i32 offset;

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
		leaderboardManager    = new LeaderboardManager ();
		leaderboardManager->LoadLeaderboard (id, difficulty, filter);

		personalLeaderboardManager                       = new LeaderboardManager ();
		personalLeaderboardManager->gettingPersonalScore = true;
		personalLeaderboardManager->LoadLeaderboard (id, difficulty, 1);

		AetLayerArgs base ("AET_PS4_GALLERY_MAIN", "hiscore_base", 0x13, AetAction::IN_LOOP);
		base.play (&hiscore_base);

		AetLayerArgs my ("AET_PS4_GALLERY_MAIN", "hiscore_my_base", 0x13, AetAction::IN_LOOP);
		my.play (&my_hiscore);

		AetLayerArgs load ("AET_PS4_GALLERY_MAIN", "hiscore_load", 0x15, AetAction::IN_LOOP);
		load.play (&hiscore_load);

		AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
		up.play (&arrow_up);

		AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
		down.play (&arrow_down);

		char buf[64];
		AetComposition comp;
		GetComposition (&comp, hiscore_base);
		for (int i = 0; i < 10; i++) {
			sprintf (buf, "p_hiscore_base%02d_c", i + 1);
			if (auto layout = comp.find (string (buf))) {
				AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", "hiscore_base_03", 0x14, AetAction::NONE);
				entry.position = layout.value ()->position;
				entry.play (&hiscore_entries[i]);
			}
		}
	}

	if (state == 2) {
		leaderboardManager->Update ();
		auto scores = leaderboardManager->GetScores ();
		if (leaderboardManager->HasFailed ()) {
			StopAet (&hiscore_load);
			*(i32 *)(task + 0x6C) = 3;

			AetLayerArgs me_none ("AET_PS4_GALLERY_MAIN", "hiscore_my_none", 0x15, AetAction::LOOP);
			me_none.play (&my_hiscore_none);

			char buf[64];
			AetComposition comp;
			GetComposition (&comp, hiscore_base);
			for (u64 i = 0; i < 10; i++) {
				sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
				if (auto layout = comp.find (string (buf))) {
					AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", "hiscore_base_03", 0x14, AetAction::NONE);
					entry.position = layout.value ()->position;
					entry.play (&hiscore_entries[i]);
				}
			}
		} else if (scores.has_value ()) {
			StopAet (&hiscore_load);
			*(i32 *)(task + 0x6C) = 3;

			if (scores.value ().size () > 0) {
				AetLayerArgs cursor ("AET_PS4_GALLERY_MAIN", "hiscore_base_cursor", 0x14, AetAction::LOOP);
				cursor.play (&hiscore_cursor);
			}

			AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
			up.play (&arrow_up);

			if (scores.value ().size () > 10) {
				AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::LOOP);
				down.play (&arrow_down);
			} else {
				AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
				down.play (&arrow_down);
			}

			if (!personalLeaderboardManager->GetScores ().has_value () || personalLeaderboardManager->GetScores ().value ().size () == 0) {
				AetLayerArgs me_none ("AET_PS4_GALLERY_MAIN", "hiscore_my_none", 0x15, AetAction::LOOP);
				me_none.play (&my_hiscore_none);
			} else {
				StopAet (&my_hiscore_none);
			}

			char buf[64];
			AetComposition comp;
			GetComposition (&comp, hiscore_base);
			for (u64 i = 0; i < 10; i++) {
				sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
				if (auto layout = comp.find (string (buf))) {
					const char *name = scores.value ().size () <= i ? "hiscore_base_03" : i % 2 == 0 ? "hiscore_base_01" : "hiscore_base_02";
					AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", name, 0x14, AetAction::NONE);
					entry.position = layout.value ()->position;
					entry.color.w  = layout.value ()->opacity;
					entry.play (&hiscore_entries[i]);
				}
			}
		}
	}

	if (state == 3 && leaderboardManager != nullptr) {
		if (difficulty != lastDiff || filter != lastFilter) {
			*(i32 *)(task + 0x6C) = 2;
			leaderboardManager->LoadLeaderboard (id, difficulty, filter);
			personalLeaderboardManager->LoadLeaderboard (id, difficulty, 1);
			AetLayerArgs load ("AET_PS4_GALLERY_MAIN", "hiscore_load", 0x15, AetAction::IN_LOOP);
			load.play (&hiscore_load);
			StopAet (&hiscore_cursor);

			currentSelected      = 0;
			currentLocalSelected = 0;
			offset               = 0;
		}

		lastDiff   = difficulty;
		lastFilter = filter;

		auto scores = leaderboardManager->GetScores ();
		if (scores.has_value ()) {
			void *inputState = diva::GetInputState (0);
			if (IsButtonDown (inputState, Button::UP) && currentSelected != 0) {
				currentSelected -= 1;

				if (currentLocalSelected == 0) {
					currentLocalSelected = 9;
					offset -= 10;

					AetLayerArgs base ("AET_PS4_GALLERY_MAIN", "hiscore_base_up", 0x13, AetAction::IN_LOOP);
					base.play (&hiscore_base);

					if (offset == 0) {
						AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::IN_ONCE);
						up.play (&arrow_up);
					} else {
						AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::SPECIAL_LOOP);
						up.play (&arrow_up);
					}

					AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::LOOP);
					down.play (&arrow_down);

					char buf[64];
					AetComposition comp;
					GetComposition (&comp, hiscore_base);
					for (u64 i = 0; i < 10; i++) {
						sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
						if (auto layout = comp.find (string (buf))) {
							const char *name = scores.value ().size () <= i + offset ? "hiscore_base_03" : i % 2 == 0 ? "hiscore_base_01" : "hiscore_base_02";
							AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", name, 0x14, AetAction::NONE);
							entry.position = layout.value ()->position;
							entry.color.w  = layout.value ()->opacity;
							entry.play (&hiscore_entries[i]);
						}
					}
				} else currentLocalSelected -= 1;
			} else if (IsButtonDown (inputState, Button::DOWN) && currentSelected != scores.value ().size () - 1) {
				currentSelected += 1;

				if (currentLocalSelected == 9) {
					currentLocalSelected = 0;
					offset += 10;

					AetLayerArgs base ("AET_PS4_GALLERY_MAIN", "hiscore_base_down", 0x13, AetAction::IN_LOOP);
					base.play (&hiscore_base);

					AetLayerArgs up ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_u", 0x13, AetAction::LOOP);
					up.play (&arrow_up);

					if (offset + 10 >= scores.value ().size () - 1) {
						AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::IN_ONCE);
						down.play (&arrow_down);
					} else {
						AetLayerArgs down ("AET_PS4_GALLERY_MAIN", "hiscore_base_arrow_d", 0x13, AetAction::SPECIAL_LOOP);
						down.play (&arrow_down);
					}

					char buf[64];
					AetComposition comp;
					GetComposition (&comp, hiscore_base);
					for (u64 i = 0; i < 10; i++) {
						sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
						if (auto layout = comp.find (string (buf))) {
							const char *name = scores.value ().size () <= i + offset ? "hiscore_base_03" : i % 2 == 0 ? "hiscore_base_01" : "hiscore_base_02";
							AetLayerArgs entry ("AET_PS4_GALLERY_MAIN", name, 0x14, AetAction::NONE);
							entry.position = layout.value ()->position;
							entry.color.w  = layout.value ()->opacity;
							entry.play (&hiscore_entries[i]);
						}
					}
				} else currentLocalSelected += 1;
			}
		}
	}

	if (state == 1 && leaderboardManager != nullptr) {
		delete leaderboardManager;
		leaderboardManager = nullptr;

		delete leaderboardManager;
		personalLeaderboardManager = nullptr;

		currentSelected      = 0;
		currentLocalSelected = 0;
		offset               = 0;

		StopAet (&hiscore_base);
		StopAet (&hiscore_cursor);
		StopAet (&my_hiscore);
		StopAet (&my_hiscore_none);
		StopAet (&hiscore_load);
		for (int i = 0; i < 10; i++)
			StopAet (&hiscore_entries[i]);
		StopAet (&arrow_up);
		StopAet (&arrow_down);
	}

	return false;
}

bool
HiScoreDestroy (u64 task) {
	return false;
}

bool
HiScoreDisplay (u64 task) {
	i32 state = *(i32 *)(task + 0x6C);
	if (state == 2 || state == 3) {
		FontInfo font;
		GetLangFont (&font, FontId::FNT_36_DIVA_36_38, true);
		SetFontSize (&font, 20.0, 30.0);

		DrawParams params;
		params.font           = &font;
		params.layer          = 0x16;
		params.resolutionMode = RESOLUTION_MODE_FHD;
		char buf[64];
		AetComposition comp;
		GetComposition (&comp, hiscore_base);

		sprintf (buf, "p_hiscore_base%02d_c", currentLocalSelected + 1);
		auto layer  = aets->find (hiscore_cursor);
		auto layout = comp.find (string (buf));
		if (layout.has_value () && layer.has_value ()) {
			layer.value ()->position = layout.value ()->position;
			layer.value ()->color.w  = layout.value ()->opacity;
		}

		for (u64 i = 0; i < 10; i++) {
			sprintf (buf, "p_hiscore_base%02lld_c", i + 1);
			auto layer  = aets->find (hiscore_entries[i]);
			auto layout = comp.find (string (buf));
			if (!layout.has_value () || !layer.has_value ()) continue;
			layer.value ()->position = layout.value ()->position;
			layer.value ()->color.w  = layout.value ()->opacity;

			if (leaderboardManager == nullptr) continue;
			auto scores = leaderboardManager->GetScores ();
			if (!scores.has_value () || scores.value ().size () <= i + offset) continue;
			auto score = scores.value ().at (i + offset);

			GetSpriteFont (&font, 0xBE37, 38, 46);
			SetFontSize (&font, 26.0, 30.0);
			params.colour[0] = 0xFF;
			params.colour[1] = 0xFF;
			params.colour[2] = 0xFF;

			sprintf (buf, "p_hiscore_score%02lld_rt", i + 1);
			if (auto layout = comp.find (string (buf))) {
				auto position      = Vec2 (layout.value ()->position.x - layout.value ()->width * 2.0, layout.value ()->position.y + layout.value ()->height / 3.0);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%06d", score.score);
			}

			sprintf (buf, "p_hiscore_grade%02lld_c", i + 1);
			if (auto layout = comp.find (string (buf))) {
				auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%d", score.rank);
			}

			sprintf (buf, "p_hiscore_rank%02lld_c", i + 1);
			if (auto layout = comp.find (string (buf))) {
				auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, "%02d", score.playerRank);
			}

			GetLangFont (&font, FontId::FNT_36_DIVA_36_38, true);
			SetFontSize (&font, 36.0, 38.0);
			params.colour[0] = 0x36;
			params.colour[1] = 0x46;
			params.colour[2] = 0x49;

			sprintf (buf, "p_hiscore_id%02lld_c", i + 1);
			if (auto layout = comp.find (string (buf))) {
				auto position      = Vec2 (layout.value ()->position.x, layout.value ()->position.y);
				params.lineOrigin  = position;
				params.textCurrent = position;
				params.colour[3]   = layout.value ()->opacity * 0xFF;
				DrawTextFmt (&params, 0x28, score.playerName.c_str ());
			}
		}

		if (personalLeaderboardManager != nullptr && personalLeaderboardManager->GetScores ().has_value () && personalLeaderboardManager->GetScores ().value ().size () == 1) {
			StopAet (&my_hiscore_none);
			GetSpriteFont (&font, 0xBE37, 38, 46);
			params.colour[0] = 0xFF;
			params.colour[1] = 0xFF;
			params.colour[2] = 0xFF;
			SetFontSize (&font, 26.0, 30.0);

			GetComposition (&comp, my_hiscore);

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

void
init () {
	taskAddition addition;
	addition.loop    = HiScoreLoop;
	addition.destroy = HiScoreDestroy;
	addition.display = HiScoreDisplay;
	addTaskAddition ("CS_RANKING_HI_SCORE", addition);
}

} // namespace leaderboard