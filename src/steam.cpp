#include "steam.h"

namespace steam {
void
LeaderboardDownloadManager::ScoreManager::FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure) {
	if (failure || !res->m_bLeaderboardFound) {
		this->state = 0;
		return;
	}

	this->leaderboard = res->m_hSteamLeaderboard;
	i32 start         = 1;
	i32 end           = 1000;
	if (this->filter == k_ELeaderboardDataRequestGlobalAroundUser) {
		if (getSingleResult) {
			start = 0;
			end   = 0;
		} else {
			start = -4;
			end   = 5;
		}
	}
	auto callback = SteamUserStats ()->DownloadLeaderboardEntries (this->leaderboard, this->filter, start, end);
	this->m_LeaderboardDownloadResult.Set (callback, this, &LeaderboardDownloadManager::ScoreManager::DownloadLeaderboardCallback);
	this->state = -1;
}

void
LeaderboardDownloadManager::ScoreManager::DownloadLeaderboardCallback (LeaderboardScoresDownloaded_t *res, bool failure) {
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

void
LeaderboardDownloadManager::ScoreManager::OnPersonaStateChange (PersonaStateChange_t *ret) {
	if (this->scores.size () == 0) return;
	for (u64 i = 0; i < this->scores.size (); i++) {
		if (this->scores[i].playerId != ret->m_ulSteamID) continue;
		this->scores[i].playerName = std::string (SteamFriends ()->GetFriendPersonaName (ret->m_ulSteamID));
		break;
	}

	u64 i = 0;
	for (; i < scores.size (); i++)
		if (this->scores[i].playerName.size () == 0) break;

	if (i == this->scores.size ()) this->state = 2;
	else this->state = -1;
}

void
LeaderboardDownloadManager::AchievementManager::FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure) {
	if (failure || !res->m_bLeaderboardFound) {
		this->state = 0;
		return;
	}

	this->leaderboard = res->m_hSteamLeaderboard;
	i32 start         = 1;
	i32 end           = 1000;
	if (this->filter == k_ELeaderboardDataRequestGlobalAroundUser) {
		if (getSingleResult) {
			start = 0;
			end   = 0;
		} else {
			start = -4;
			end   = 5;
		}
	}
	auto callback = SteamUserStats ()->DownloadLeaderboardEntries (this->leaderboard, this->filter, start, end);
	this->m_LeaderboardDownloadResult.Set (callback, this, &LeaderboardDownloadManager::AchievementManager::DownloadLeaderboardCallback);
	this->state = -1;
}

void
LeaderboardDownloadManager::AchievementManager::DownloadLeaderboardCallback (LeaderboardScoresDownloaded_t *res, bool failure) {
	if (failure) {
		this->state = 1;
		return;
	}

	this->achievements.clear ();
	for (int i = 0; i < res->m_cEntryCount; i++) {
		LeaderboardEntry_t entry;
		i32 details[2];
		SteamUserStats ()->GetDownloadedLeaderboardEntry (res->m_hSteamLeaderboardEntries, i, &entry, details, 2);

		Achievement achievement;
		achievement.rank               = entry.m_nGlobalRank;
		achievement.combinedPercentage = std::bit_cast<f32> (entry.m_nScore);
		achievement.playerId           = entry.m_steamIDUser.ConvertToUint64 ();
		achievement.playerRank         = details[0];

		if (SteamFriends ()->RequestUserInformation (entry.m_steamIDUser, true) == false) achievement.playerName = std::string (SteamFriends ()->GetFriendPersonaName (entry.m_steamIDUser));
		else achievement.playerName = std::string ("");

		this->achievements.push_back (achievement);
	}

	u64 i = 0;
	for (; i < this->achievements.size (); i++)
		if (this->achievements[i].playerName.size () == 0) break;

	if (i == this->achievements.size ()) this->state = 2;
	else this->state = -1;
}

void
LeaderboardDownloadManager::AchievementManager::OnPersonaStateChange (PersonaStateChange_t *ret) {
	if (this->achievements.size () == 0) return;
	for (u64 i = 0; i < this->achievements.size (); i++) {
		if (this->achievements[i].playerId != ret->m_ulSteamID) continue;
		this->achievements[i].playerName = std::string (SteamFriends ()->GetFriendPersonaName (ret->m_ulSteamID));
		break;
	}

	u64 i = 0;
	for (; i < achievements.size (); i++)
		if (this->achievements[i].playerName.size () == 0) break;

	if (i == this->achievements.size ()) this->state = 2;
	else this->state = -1;
}

void
LeaderboardDownloadManager::DownloadScores (i32 id, i32 diff, i32 filter, bool getSingleResult) {
	char buf[64];
	sprintf (buf, "SCORE_%05dD%cA%cMA", id, diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->score.m_LeaderboardFindResult.Set (callback, &this->score, &LeaderboardDownloadManager::ScoreManager::FindLeaderboardCallback);

	this->score.state           = -1;
	this->score.filter          = (ELeaderboardDataRequest)filter;
	this->score.getSingleResult = getSingleResult;
	this->score.scores.clear ();
}

void
LeaderboardDownloadManager::DownloadSurvivalScores (i32 id, i32 filter, bool getSingleResult) {
	char buf[64];
	sprintf (buf, "SURVIV_%03d", id);
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->score.m_LeaderboardFindResult.Set (callback, &this->score, &LeaderboardDownloadManager::ScoreManager::FindLeaderboardCallback);

	this->score.state           = -1;
	this->score.filter          = (ELeaderboardDataRequest)filter;
	this->score.getSingleResult = getSingleResult;
	this->score.scores.clear ();

	this->achievement.state = -1;
}

void
LeaderboardDownloadManager::DownloadAchievements (i32 diff, i32 filter, bool getSingleResult) {
	char buf[64];
	sprintf (buf, "ACHIEV_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardDownloadManager::AchievementManager::FindLeaderboardCallback);

	this->achievement.state           = -1;
	this->achievement.filter          = (ELeaderboardDataRequest)filter;
	this->achievement.getSingleResult = getSingleResult;
	this->achievement.achievements.clear ();

	this->score.state = -1;
}

void
LeaderboardDownloadManager::DownloadFSAchievements (i32 diff, i32 filter, bool getSingleResult) {
	char buf[64];
	sprintf (buf, "ACHIEV_FS_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardDownloadManager::AchievementManager::FindLeaderboardCallback);

	this->achievement.state           = -1;
	this->achievement.filter          = (ELeaderboardDataRequest)filter;
	this->achievement.getSingleResult = getSingleResult;
	this->achievement.achievements.clear ();

	this->score.state = -1;
}

void
LeaderboardDownloadManager::DownloadCTAchievements (i32 diff, i32 filter, bool getSingleResult) {
	char buf[64];
	sprintf (buf, "ACHIEV_CT_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardDownloadManager::AchievementManager::FindLeaderboardCallback);

	this->achievement.state           = -1;
	this->achievement.filter          = (ELeaderboardDataRequest)filter;
	this->achievement.getSingleResult = getSingleResult;
	this->achievement.achievements.clear ();

	this->score.state = -1;
}

void
LeaderboardDownloadManager::Update () {
	SteamAPI_RunCallbacks ();
}

bool
LeaderboardDownloadManager::HasFailed () {
	return this->score.state == 0 || this->score.state == 1 || this->achievement.state == 0 || this->achievement.state == 1;
}

bool
LeaderboardDownloadManager::HasSucceeded () {
	return this->score.state == 2 || this->achievement.state == 2;
}

std::optional<std::vector<Score>>
LeaderboardDownloadManager::GetScores () {
	if (this->score.state == 2) return this->score.scores;
	else return {};
}

std::optional<std::vector<Achievement>>
LeaderboardDownloadManager::GetAchievements () {
	if (this->achievement.state == 2) return this->achievement.achievements;
	else return {};
}

void
LeaderboardUploadManager::ScoreManager::FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure) {
	if (failure || !res->m_bLeaderboardFound) {
		this->finished = true;
		return;
	}

	i32 details[2] = {this->rank, 1};

	auto callback = SteamUserStats ()->UploadLeaderboardScore (res->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, this->score, details, 2);
	this->m_LeaderboardUploadResult.Set (callback, this, &LeaderboardUploadManager::ScoreManager::UploadCallback);
}

void
LeaderboardUploadManager::ScoreManager::UploadCallback (LeaderboardScoreUploaded_t *res, bool failure) {
	this->finished = true;
}

void
LeaderboardUploadManager::AchievementManager::FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure) {
	if (failure || !res->m_bLeaderboardFound) return;

	i32 details[2] = {this->rank, 1};

	auto callback = SteamUserStats ()->UploadLeaderboardScore (res->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, std::bit_cast<i32> (this->combinedPercentage), details, 2);
	this->m_LeaderboardUploadResult.Set (callback, this, &LeaderboardUploadManager::AchievementManager::UploadCallback);
}

void
LeaderboardUploadManager::AchievementManager::UploadCallback (LeaderboardScoreUploaded_t *res, bool failure) {
	this->finished = true;
}

void
LeaderboardUploadManager::UploadScore (i32 id, i32 diff, u32 score, u32 rank) {
	char buf[64];
	sprintf (buf, "SCORE_%05dD%cA%cMA", id, diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->score.m_LeaderboardFindResult.Set (callback, &this->score, &LeaderboardUploadManager::ScoreManager::FindLeaderboardCallback);
	this->score.score          = score;
	this->score.rank           = rank;
	this->score.finished       = false;
	this->achievement.finished = false;
}

void
LeaderboardUploadManager::UploadSurvival (i32 id, u32 score, u32 rank) {
	char buf[64];
	sprintf (buf, "SURVIV_%03d", id);
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->score.m_LeaderboardFindResult.Set (callback, &this->score, &LeaderboardUploadManager::ScoreManager::FindLeaderboardCallback);
	this->score.score          = score;
	this->score.rank           = rank;
	this->score.finished       = false;
	this->achievement.finished = false;
}

void
LeaderboardUploadManager::UploadAchievement (i32 diff, f32 combinedPercentage, i32 rank) {
	char buf[64];
	sprintf (buf, "ACHIEV_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardUploadManager::AchievementManager::FindLeaderboardCallback);
	this->achievement.combinedPercentage = combinedPercentage;
	this->achievement.rank               = rank;
	this->achievement.finished           = false;
	this->score.finished                 = false;
}

void
LeaderboardUploadManager::UploadFSAchievement (i32 diff, f32 combinedPercentage, i32 rank) {
	char buf[64];
	sprintf (buf, "ACHIEV_FS_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardUploadManager::AchievementManager::FindLeaderboardCallback);
	this->achievement.combinedPercentage = combinedPercentage;
	this->achievement.rank               = rank;
	this->achievement.finished           = false;
	this->score.finished                 = false;
}

void
LeaderboardUploadManager::UploadCTAchievement (i32 diff, f32 combinedPercentage, i32 rank) {
	char buf[64];
	sprintf (buf, "ACHIEV_CT_D%cA%cMA", diff > 0 ? 'X' : 'H', diff == 2 ? 'E' : 'O');
	auto callback = SteamUserStats ()->FindOrCreateLeaderboard (buf, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
	this->achievement.m_LeaderboardFindResult.Set (callback, &this->achievement, &LeaderboardUploadManager::AchievementManager::FindLeaderboardCallback);
	this->achievement.combinedPercentage = combinedPercentage;
	this->achievement.rank               = rank;
	this->achievement.finished           = false;
	this->score.finished                 = false;
}

void
LeaderboardUploadManager::Update () {
	SteamAPI_RunCallbacks ();
}

bool
LeaderboardUploadManager::HasFinished () {
	return this->score.finished || this->achievement.finished;
}

} // namespace steam