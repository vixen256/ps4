#include "diva.h"
#include "menus.h"
#include "steam.h"

namespace result {
using namespace diva;
using namespace steam;

i32 fsRank;
i32 ctRank;
i32 fsPoints;
i32 ctPoints;
i32 gainedPoints;

LeaderboardUploadManager *uploadManager = nullptr;

bool waitingAchievementUpload;
bool waitingPackUpload;

HOOK (u64, GetStageResultSwitch, 0x14064BF50) { return 0x1412C1F00; }
HOOK (bool, StageResultSwitchFinished, 0x14064C0D0, u64 task) {
	return *(i32 *)(task + 0x68) == 0x5A && !waitingAchievementUpload && !waitingPackUpload && (uploadManager == nullptr || uploadManager->HasFinished ());
}

i32 oldScore = 0;

bool
StageResultInit (u64 task) {
	waitingAchievementUpload = false;
	waitingPackUpload        = false;

	if (*(bool *)(0x1412B50A5) && !GetModuleHandleA ("LeaderboardBlocker.dll")) {
		if (IsSurvival ()) {
			if (SurvivalCleared ()) {
				auto index = *(i32 *)0x140DAB3A8;
				auto id    = 0;
				if (pvSel::survivalIndexIds.contains (index)) id = pvSel::survivalIndexIds.find (index)->second;
				else id = index;

				auto scores    = (vector<pair<i32, i32>> *)0x140DAB390;
				i32 totalScore = 0;
				for (auto it = scores->begin (); it != scores->end (); it++)
					totalScore += it->key;

				i32 exp  = 0;
				i32 rank = 0;
				GetPlayerRank (&exp, &rank);

				uploadManager = new LeaderboardUploadManager;
				uploadManager->UploadSurvival (id, totalScore, rank);
			}
		} else {
			auto pvData = GetPvLoadData ();
			auto diff   = *(i32 *)(pvData + 0x00);
			auto extra  = *(i32 *)(pvData + 0x04);
			auto pvId   = *(i32 *)(pvData + 0x20);

			auto score    = FindScore (GetSaveData (), pvId);
			auto newScore = *(i32 *)((u64)score + (0x120 * (diff + extra + 1)) - 0x10 - 0x04);

			if (diff + extra - 2 >= 0 && newScore > oldScore && newScore > 0) {
				i32 exp  = 0;
				i32 rank = 0;
				GetPlayerRank (&exp, &rank);

				uploadManager = new LeaderboardUploadManager;
				uploadManager->UploadScore (pvId, diff + extra - 2, newScore, rank);

				waitingAchievementUpload = true;
				if (auto pv = getPvDbEntry (pvId)) {
					if (pv.value ()->pack != 0) waitingPackUpload = true;
				}
			}
		}
	}

	return false;
}

bool
StageResultLoop (u64 task) {
	if (uploadManager != nullptr) {
		uploadManager->Update ();

		if (uploadManager->HasFinished ()) {
			if (waitingAchievementUpload) {
				waitingAchievementUpload = false;

				auto pvData = GetPvLoadData ();
				auto diff   = *(i32 *)(pvData + 0x00);
				auto extra  = *(i32 *)(pvData + 0x04);

				i32 exp  = 0;
				i32 rank = 0;
				GetPlayerRank (&exp, &rank);

				f32 combinedPercentage = 0.0;

				for (auto it = pvs->begin (); it != pvs->end (); it++) {
					auto score = FindScore (GetSaveData (), (*it)->id);
					if (score != 0) {
						auto percentage = *(i32 *)((u64)score + (0x120 * (diff + extra + 1)) - 0x10 + 0x04);
						combinedPercentage += (f32)percentage / 100.0;
					}
				}

				uploadManager->UploadAchievement (diff + extra - 2, combinedPercentage, rank);
			} else if (waitingPackUpload) {
				waitingPackUpload = false;

				auto pvData = GetPvLoadData ();
				auto diff   = *(i32 *)(pvData + 0x00);
				auto extra  = *(i32 *)(pvData + 0x04);
				auto pvId   = *(i32 *)(pvData + 0x20);

				i32 exp  = 0;
				i32 rank = 0;
				GetPlayerRank (&exp, &rank);

				f32 combinedPercentage = 0.0;

				if (auto pv = getPvDbEntry (pvId)) {
					for (auto it = pvs->begin (); it != pvs->end (); it++) {
						if ((*it)->pack != pv.value ()->pack) continue;
						auto score = FindScore (GetSaveData (), (*it)->id);
						if (score != 0) {
							auto percentage = *(i32 *)((u64)score + (0x120 * (diff + extra + 1)) - 0x10 + 0x04);
							combinedPercentage += (f32)percentage / 100.0;
						}
					}

					if (pv.value ()->pack == 1) uploadManager->UploadFSAchievement (diff + extra - 2, combinedPercentage, rank);
					else if (pv.value ()->pack == 2) uploadManager->UploadCTAchievement (diff + extra - 2, combinedPercentage, rank);
				}
			}
		}
	}
	return false;
}

bool
StageResultDestroy (u64 task) {
	if (uploadManager != nullptr) {
		delete uploadManager;
		uploadManager = nullptr;
	}
	return false;
}

bool
GameResultLoop (u64 task) {
	if (IsSurvival () && (LifeGauge () == 0 || SurvivalCleared ()) && *(i32 *)(task + 0x68) < 8) {
		*(i32 *)(task + 0x68) = 8;
		*(i32 *)(task + 0x88) = 1;
	}

	return false;
}

bool
PVGameInit (u64 task) {
	GetFSCTRankData (&fsRank, &ctRank, &fsPoints, &ctPoints);

	auto pvData = GetPvLoadData ();
	auto diff   = *(i32 *)(pvData + 0x00);
	auto extra  = *(i32 *)(pvData + 0x04);
	auto pvId   = *(i32 *)(pvData + 0x20);

	auto score = FindScore (GetSaveData (), pvId);
	oldScore   = *(i32 *)((u64)score + (0x120 * (diff + extra + 1)) - 0x10 - 0x04);

	return false;
}

HOOK (i32, GetPoints, 0x15DE80210, i32 a1, i32 a2, i32 a3) {
	gainedPoints = originalGetPoints (a1, a2, a3);
	return gainedPoints;
}

HOOK (i32 *, GetRankData, 0x1401E7C50) {
	PvDbEntry **pvData;
	asm ("mov %0, qword ptr [rbx + 0x100]" : "=g"(pvData));
	i32 ps4DlcType = (*pvData)->pack;

	bool isFutureSound = ps4DlcType == 1;
	bool isPs4Dlc      = ps4DlcType != 0;

	i32 rank;
	i32 points;
	i32 *pointsRequired = (i32 *)0x1412B6828;
	pointsRequired += (39 * isFutureSound);
	if (isFutureSound) {
		rank   = fsRank;
		points = fsPoints;
	} else {
		rank   = ctRank;
		points = ctPoints;
	}

	i32 *data = originalGetRankData ();

	if (isPs4Dlc) {
		data[0]  = gainedPoints;             // Gained points
		data[1]  = points;                   // Current points
		data[2]  = rank;                     // Current rank
		data[4]  = pointsRequired[rank - 1]; // Points required for current rank
		data[5]  = pointsRequired[rank];     // Points required for next rank
		data[14] = data[0] > 0;              // Has gained points
	} else {
		data[0]  = 0;
		data[1]  = 0;
		data[2]  = rank;
		data[4]  = 0;
		data[5]  = 1;
		data[14] = 0;
	}

	return data;
}

HOOK (void, PlayRankGauge, 0x1402377E0, u64 a1) {
	auto pvData = **(PvDbEntry ***)(a1 + 0x100);
	if (pvData->pack == 0) return;

	originalPlayRankGauge (a1);
}

HOOK (void, PlayRankGaugeLoop, 0x140236F80, u64 a1) {
	auto pvData = **(PvDbEntry ***)(a1 + 0x100);
	if (pvData->pack == 0) return;

	originalPlayRankGaugeLoop (a1);
}

extern "C" {
i32
getSurvivalIdForIndex (i32 index) {
	if (!pvSel::survivalIndexIds.contains (index)) return index + 1;
	else return pvSel::survivalIndexIds[index];
}

const char *
realLoadSurvivalSprite (i32 index) {
	if (!pvSel::survivalIndexIds.contains (index)) return "SPR_PS4_RESULT_COURSE_SURVIVAL_%02d";
	i32 id = pvSel::survivalIndexIds[index];
	char sprBuf[64];
	stringRange name;

	sprintf (sprBuf, "SPR_SURVIVAL_COURSE%02d_RESULT", id);
	name      = stringRange (sprBuf);
	u32 sprId = *getSpriteId (nullptr, &name);
	if (sprId == (u32)-1) return "SPR_PS4_RESULT_COURSE_SURVIVAL_MODDED";
	else return "SPR_SURVIVAL_COURSE%02d_RESULT";
}

HOOK (void, LoadSurvivalSprite, 0x140238663);
}

void
init () {
	INSTALL_HOOK (GetStageResultSwitch);
	INSTALL_HOOK (StageResultSwitchFinished);
	INSTALL_HOOK (GetPoints);
	INSTALL_HOOK (GetRankData);
	INSTALL_HOOK (PlayRankGauge);
	INSTALL_HOOK (PlayRankGaugeLoop);
	INSTALL_HOOK (LoadSurvivalSprite);

	diva::taskAddition gameResultAddition;
	gameResultAddition.loop = GameResultLoop;
	diva::addTaskAddition ("GameResultTask", gameResultAddition);

	diva::taskAddition pvGameAddition;
	pvGameAddition.init = PVGameInit;
	diva::addTaskAddition ("PVGAME", pvGameAddition);

	taskAddition resultAddition;
	resultAddition.init    = StageResultInit;
	resultAddition.loop    = StageResultLoop;
	resultAddition.destroy = StageResultDestroy;
	addTaskAddition ("STAGE_RESULT_SWITCH", resultAddition);

	WRITE_MEMORY (0x14064C0C3, u8, 0x68); // Fix game not going into results
}
} // namespace result
