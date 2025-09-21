namespace nc {
enum GameStyle : int32_t { GameStyle_Arcade = 0, GameStyle_Console = 1, GameStyle_Mixed = 2, GameStyle_Max };

extern "C" {
typedef bool (*CheckSongHasStyleAvailable) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef void *(*FindSongEntry) (i32 pv);
typedef void *(FindDifficultyEntry)(i32 pv, i32 difficulty, i32 edition);
typedef void *(*FindChart) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef bool (*SetStateSong) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef void (*ResetStateSong) ();
typedef bool (*DbReady) ();
}
} // namespace nc
