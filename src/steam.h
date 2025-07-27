namespace steam {
struct Score {
	i32 rank;
	i32 score;
	u64 playerId;
	std::string playerName;
	i32 playerRank;
};

struct Achievement {
	i32 rank;
	f32 combinedPercentage;
	u64 playerId;
	std::string playerName;
	i32 playerRank;
};

class LeaderboardDownloadManager {
private:
	class ScoreManager {
	public:
		i32 state = -1;
		ELeaderboardDataRequest filter;
		bool getSingleResult;
		SteamLeaderboard_t leaderboard;
		std::vector<Score> scores;

		CCallResult<LeaderboardDownloadManager::ScoreManager, LeaderboardFindResult_t> m_LeaderboardFindResult;
		CCallResult<LeaderboardDownloadManager::ScoreManager, LeaderboardScoresDownloaded_t> m_LeaderboardDownloadResult;

		void FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure);
		void DownloadLeaderboardCallback (LeaderboardScoresDownloaded_t *res, bool failure);
		STEAM_CALLBACK (LeaderboardDownloadManager::ScoreManager, OnPersonaStateChange, PersonaStateChange_t);
	} score;

	class AchievementManager {
	public:
		i32 state = -1;
		ELeaderboardDataRequest filter;
		bool getSingleResult;
		SteamLeaderboard_t leaderboard;
		std::vector<Achievement> achievements;

		CCallResult<LeaderboardDownloadManager::AchievementManager, LeaderboardFindResult_t> m_LeaderboardFindResult;
		CCallResult<LeaderboardDownloadManager::AchievementManager, LeaderboardScoresDownloaded_t> m_LeaderboardDownloadResult;

		void FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure);
		void DownloadLeaderboardCallback (LeaderboardScoresDownloaded_t *res, bool failure);
		STEAM_CALLBACK (LeaderboardDownloadManager::AchievementManager, OnPersonaStateChange, PersonaStateChange_t);
	} achievement;

public:
	void DownloadScores (i32 id, i32 diff, i32 filter, bool getSingleResult);
	void DownloadSurvivalScores (i32 id, i32 filter, bool getSingleResult);
	void DownloadAchievements (i32 diff, i32 filter, bool getSingleResult);
	void DownloadFSAchievements (i32 diff, i32 filter, bool getSingleResult);
	void DownloadCTAchievements (i32 diff, i32 filter, bool getSingleResult);
	void Update ();
	bool HasFailed ();
	bool HasSucceeded ();
	std::optional<std::vector<Score>> GetScores ();
	std::optional<std::vector<Achievement>> GetAchievements ();
};

class LeaderboardUploadManager {
private:
	class ScoreManager {
	public:
		i32 score;
		i32 rank;
		bool finished = true;

		CCallResult<LeaderboardUploadManager::ScoreManager, LeaderboardFindResult_t> m_LeaderboardFindResult;
		CCallResult<LeaderboardUploadManager::ScoreManager, LeaderboardScoreUploaded_t> m_LeaderboardUploadResult;

		void FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure);
		void UploadCallback (LeaderboardScoreUploaded_t *res, bool failure);
	} score;

	class AchievementManager {
	public:
		f32 combinedPercentage;
		i32 rank;
		bool finished = true;

		CCallResult<LeaderboardUploadManager::AchievementManager, LeaderboardFindResult_t> m_LeaderboardFindResult;
		CCallResult<LeaderboardUploadManager::AchievementManager, LeaderboardScoreUploaded_t> m_LeaderboardUploadResult;

		void FindLeaderboardCallback (LeaderboardFindResult_t *res, bool failure);
		void UploadCallback (LeaderboardScoreUploaded_t *res, bool failure);
	} achievement;

public:
	void UploadScore (i32 id, i32 diff, u32 score, u32 rank);
	void UploadSurvival (i32 id, u32 score, u32 rank);
	void UploadAchievement (i32 diff, f32 combinedPercentage, i32 rank);
	void UploadFSAchievement (i32 diff, f32 combinedPercentage, i32 rank);
	void UploadCTAchievement (i32 diff, f32 combinedPercentage, i32 rank);
	void Update ();
	bool HasFinished ();
};
} // namespace steam
