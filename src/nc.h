#include "diva.h"

namespace nc {
#pragma pack(push, 8)
enum GameStyle : int32_t { GameStyle_Arcade = 0, GameStyle_Console = 1, GameStyle_Mixed = 2, GameStyle_Max };
constexpr size_t MaxChartsPerDifficulty = 3;
constexpr size_t MaxDifficultyCount     = 5;
constexpr size_t MaxEditionCount        = 2;
constexpr const char *DefaultStarSound  = "scratch1_mmv";
constexpr const char *DefaultCopySound  = "(TARGET)";

struct ChartAttributes {
	bool has_hold   = false;
	bool has_multi  = false;
	bool has_slide  = false;
	bool has_double = false;
	bool has_long   = false;
	bool has_rush   = false;
	bool has_star   = false;
};

struct ChartEntry {
	int32_t style                 = GameStyle_Max;
	int32_t difficulty_level      = 0;
	ChartAttributes attributes    = {};
	diva::string script_file_name = "(NULL)";
};

struct DifficultyEntry {
	diva::vector<ChartEntry> charts;
};

struct SongEntry {
	diva::string star_se_name      = DefaultStarSound;
	diva::string double_se_name    = DefaultCopySound;
	diva::string long_se_name      = DefaultCopySound;
	diva::string star_long_se_name = DefaultStarSound;
	diva::string star_w_se_name    = DefaultStarSound;
	diva::string link_se_name      = DefaultStarSound;
	diva::array<diva::optional<DifficultyEntry>, MaxDifficultyCount * MaxEditionCount> difficulties;
};
#pragma pack(pop)

extern "C" {
typedef bool (*CheckSongHasStyleAvailable) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef SongEntry *(*FindSongEntry) (i32 pv);
typedef DifficultyEntry *(FindDifficultyEntry)(i32 pv, i32 difficulty, i32 edition);
typedef ChartEntry *(*FindChart) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef bool (*SetStateSong) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef void (*ResetStateSong) ();
typedef bool (*DbReady) ();
}
} // namespace nc
