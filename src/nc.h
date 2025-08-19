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

enum TargetType : int32_t {
	// FT
	TargetType_Triangle       = 0,
	TargetType_Circle         = 1,
	TargetType_Cross          = 2,
	TargetType_Square         = 3,
	TargetType_TriangleHold   = 4,
	TargetType_CircleHold     = 5,
	TargetType_CrossHold      = 6,
	TargetType_SquareHold     = 7,
	TargetType_Random         = 8,
	TargetType_RandomHold     = 9,
	TargetType_Previous       = 10,
	TargetType_0B             = 11,
	TargetType_SlideL         = 12,
	TargetType_SlideR         = 13,
	TargetType_0E             = 14,
	TargetType_ChainslideL    = 15,
	TargetType_ChainslideR    = 16,
	TargetType_11             = 17,
	TargetType_ChanceTriangle = 18,
	TargetType_ChanceCircle   = 19,
	TargetType_ChanceCross    = 20,
	TargetType_ChanceSquare   = 21,
	TargetType_16             = 22,
	TargetType_ChanceSlideL   = 23,
	TargetType_ChanceSlideR   = 24,

	// X (these are their actual IDs)
	TargetType_TriangleRush = 25,
	TargetType_CircleRush   = 26,
	TargetType_CrossRush    = 27,
	TargetType_SquareRush   = 28,

	// PSP / F / F 2nd (Changed IDs so they don't overlap base game notes)
	TargetType_UpW          = 29,
	TargetType_RightW       = 30,
	TargetType_DownW        = 31,
	TargetType_LeftW        = 32,
	TargetType_TriangleLong = 33,
	TargetType_CircleLong   = 34,
	TargetType_CrossLong    = 35,
	TargetType_SquareLong   = 36,
	TargetType_Star         = 37,
	TargetType_StarLong     = 38, // NOTE: Unused F mechanic, should I implement this?
	TargetType_StarW        = 39,
	TargetType_ChanceStar   = 40,
	TargetType_LinkStar     = 41,
	TargetType_LinkStarEnd  = 42,
	TargetType_StarRush     = 43,

	TargetType_Max,
	TargetType_Custom = 25
};

enum LinkStepState : int32_t {
	LinkStepState_None      = 0,
	LinkStepState_FadeIn    = 1,
	LinkStepState_Normal    = 2,
	LinkStepState_GlowStart = 3,
	LinkStepState_Glow      = 4,
	LinkStepState_GlowEnd   = 5,
	LinkStepState_Wait      = 6,
	LinkStepState_Idle      = 7
};

enum SEType : int32_t {
	SEType_Normal      = 1,
	SEType_Star        = 2,
	SEType_LongStart   = 3,
	SEType_LongRelease = 4,
	SEType_LongFail    = 5,
	SEType_Double      = 6,
	SEType_Cymbal      = 7,
	SEType_StarDouble  = 8,
	SEType_RushStart   = 9,
	SEType_RushPop     = 10,
	SEType_RushFail    = 11,
	SEType_LinkStart   = 12,
	SEType_LinkEnd     = 13,
	SEType_Max         = 14
};

enum HitState : int32_t {
	HitState_Cool       = 0,
	HitState_Fine       = 1,
	HitState_Safe       = 2,
	HitState_Sad        = 3,
	HitState_WrongCool  = 4,
	HitState_WrongFine  = 5,
	HitState_WrongSafe  = 6,
	HitState_WrongSad   = 7,
	HitState_Worst      = 8,
	HitState_CoolDouble = 9,
	HitState_FineDouble = 10,
	HitState_SafeDouble = 11,
	HitState_SadDouble  = 12,
	HitState_CoolTriple = 13,
	HitState_FineTriple = 14,
	HitState_SafeTriple = 15,
	HitState_SadTriple  = 16,
	HitState_CoolQuad   = 17,
	HitState_FineQuad   = 18,
	HitState_SafeQuad   = 19,
	HitState_SadQuad    = 20,
	HitState_None       = 21
};

struct SpriteVertex {
	diva::Vec3 pos;
	diva::Vec2 uv;
	uint32_t color;
};

struct PvGameTarget {
	PvGameTarget *prev;
	PvGameTarget *next;
	int32_t dword10;
	int32_t target_type;
	float flying_time_remaining;
	float player_hit_time;
	float flying_time;
	float amplitude;
	float freq;
	float cur_freq;
	uint8_t gap30[4];
	int32_t dword34;
	uint8_t gap38[4];
	float dword3C;
	uint8_t gap40;
	bool slide_chain_start;
	bool slide_chain_end;
	bool slide_left;
	bool slide_right;
	bool slide_chain;
	uint8_t gap46[2];
	int64_t hit_time;
	uint8_t gap50[32];
	int32_t target_aet;
	int32_t button_aet;
	int32_t dword78;
	int32_t target_eff_aet;
	diva::Vec2 target_pos;
	diva::Vec2 button_pos;
	diva::Vec2 delta_pos;
	diva::Vec2 delta_pos_sq;
	diva::Vec2 vecA0;
	SpriteVertex kiseki[40];
	bool note_active;
	uint8_t gap469[3];
	float out_start_time;
	uint8_t gap470[4];
	int32_t hit_state;
	int32_t multi_count;
	int32_t dword47C;
	float float480;
	uint8_t gap484[12];
	uint8_t byte490;
	bool b491;
	bool b492;
	bool b493;
	float button_opacity;
	float target_opacity;
	float sudden_appear_frame;
	float scaling_end_time;
	uint8_t gap4A4[4];
	int32_t sprite_index;
	uint8_t gap4AC[4];
	int64_t appear_time;
	int16_t word4B8;
	uint8_t gap4BA[2];
	float kiseki_width;
	int32_t target_index;
	int32_t dword4C4;
};

struct ButtonState {
	static constexpr size_t MaxKeepStates = 32;

	struct StateData {
		bool down;
		bool up;
		bool tapped;
		bool released;
	} data[MaxKeepStates];
};

struct TargetStateEx {
	// NOTE: Static data; Information about the target.
	TargetStateEx *prev       = nullptr;
	TargetStateEx *next       = nullptr;
	int32_t target_type       = -1;
	int32_t target_index      = 0;
	int32_t sub_index         = 0;
	float length              = 0.0f;
	bool long_end             = false;
	bool link_start           = false;
	bool link_step            = false;
	bool link_end             = false;
	int32_t bal_max_hit_count = 0;

	// NOTE: Gameplay state
	ButtonState *hold_button    = nullptr;
	PvGameTarget *org           = nullptr;
	int32_t force_hit_state     = HitState_None;
	int32_t hit_state           = HitState_None;
	float hit_time              = 0.0f;
	float flying_time_max       = 0.0f;
	float flying_time_remaining = 0.0f;
	float delta_time_max        = 0.0f;
	float delta_time            = 0.0f;
	float length_remaining      = 0.0f;
	float kiseki_time           = 0.0f;
	float alpha                 = 0.0f;
	bool holding                = false;
	bool success                = false; // NOTE: If this note is a chance star, this determines if it's successful or not
	bool current_step           = false; // NOTE: If this target is the current step of the link star chain
	int32_t step_state          = LinkStepState_None;
	bool link_ending            = false;

	float sustain_bonus_time = 0.0f;
	int32_t score_bonus      = 0;
	int32_t ct_score_bonus   = 0;
	bool double_tapped       = false;
	int32_t bal_hit_count    = 0;

	// NOTE: Visual info for long notes. This is kind of a workaround as to not mess too much
	//       with the vanilla game structs.
	int32_t target_aet          = 0;
	int32_t button_aet          = 0;
	int32_t bal_effect_aet      = 0;
	diva::Vec2 target_pos       = {};
	diva::Vec2 kiseki_pos       = {}; // NOTE: Position where the kiseki will be updated from
	diva::Vec2 kiseki_dir       = {}; // NOTE: Direction of the note
	diva::Vec2 kiseki_dir_norot = {};
	diva::vector<SpriteVertex> kiseki;
	size_t vertex_count_max = 0;
	bool fix_long_kiseki    = false;
	float bal_time          = -1.0f;
	float bal_start_time    = -1.0f;
	float bal_end_time      = -1.0f;
	float bal_scale         = 0.0f;

	void ResetPlayState ();
	void ResetAetData ();
	bool IsChainSucessful ();
	void StopAet (bool button = true, bool target = true, bool kiseki = true);
	bool SetLongNoteAet ();
	bool SetLinkNoteAet ();
	bool SetRushNoteAet ();
};

struct ChanceState {
	int32_t first_target_index = -1;
	int32_t last_target_index  = -1;
	int32_t targets_hit        = 0;
	bool enabled               = false;
	bool successful            = false;
};

struct TechZoneState {
	int32_t first_target_index = -1;
	int32_t last_target_index  = -1;
	int32_t targets_hit        = 0;
	bool failed                = false;
};

struct TechZoneDispState {
	TechZoneState *data;
	uint32_t scene;
	diva::string layer_name;
	int32_t prio;
	int32_t state;
	bool end;
	bool fail_in;
};

enum LayerUI : int32_t { LayerUI_ChanceFrameTop = 0, LayerUI_ChanceFrameBottom, LayerUI_StarGaugeBase, LayerUI_StarGauge, LayerUI_ChanceTxt, LayerUI_BonusZone, LayerUI_BonusZoneText, LayerUI_Max };

struct AetArgs {
	uint32_t scene_id      = 0;
	const char *layer_name = nullptr;
	diva::string start_marker;
	diva::string end_marker;
	diva::string loop_marker;
	float start_time  = -1.0f;
	float end_time    = -1.0f;
	int32_t flags     = 0x0;
	int32_t index     = 0;
	int32_t layer     = 0;
	int32_t prio      = 0;
	int32_t res_mode  = 13;
	diva::Vec3 pos    = {0.0f, 0.0f, 0.0f};
	diva::Vec3 rot    = {0.0f, 0.0f, 0.0f};
	diva::Vec3 scale  = {1.0f, 1.0f, 1.0f};
	diva::Vec3 anchor = {0.0f, 0.0f, 0.0f};
	float frame_speed = 1.0f;
	diva::Vec4 color  = {1.0f, 1.0f, 1.0f, 1.0f};
	diva::map<diva::string, int32_t> layer_sprites;
	diva::string sound_path;
	diva::map<diva::string, diva::string> sound_replace;
	int32_t sound_queue_index = 0;
	diva::map<uint32_t, uint32_t> sprite_replace;
	diva::map<uint32_t, void *> sprite_texture;
	diva::map<uint32_t, uint32_t> sprite_discard;
	void *frame_rate_control = nullptr;
	bool sound_voice         = false;
	int32_t dword154         = 0;
	int32_t dword158         = 0;
	int32_t dword15C         = 0;
	int32_t id               = 0;
	int32_t dword164         = 0;
	diva::Vec3 pos_2         = {0.0f, 0.0f, 0.0f};

	AetArgs () = default;
	AetArgs (uint32_t scene, const char *layer, int32_t prio, int32_t marker_mode);
	~AetArgs () = default;
};

class AetElement {
public:
	virtual void Ctrl ();
	virtual void Disp ();

private:
	uint32_t scene_id = 0;
	int32_t handle    = 0;
	AetArgs args      = {};
	diva::string layer_name;

	void DeleteHandle ();
	void Remake ();
};

struct UIState {
	int32_t aet_list[LayerUI_Max];
	// TODO: Change all the layers to use AetElement
	diva::shared_ptr<AetElement> elements[LayerUI_Max];
	bool aet_visibility[LayerUI_Max];
	int32_t hit_effect_index;
};

struct ScoreState {
	float target_max_rate        = 0.0f;
	int32_t max_ct_score_bonus   = 0;
	int32_t max_sustain_bonus    = 0;
	int32_t max_double_tap_bonus = 0;
	int32_t max_link_bonus       = 0;

	int32_t ct_score_bonus   = 0;
	int32_t double_tap_bonus = 0;
	int32_t sustain_bonus    = 0;
	int32_t link_bonus       = 0;
	int32_t rush_bonus       = 0;
};

struct StateEx {
	static constexpr int32_t MaxHitEffectCount = 4;

	diva::list<TargetStateEx *> target_references;
	void *file_handler;
	int32_t file_state;
	bool dsc_loaded;
	bool files_loaded;
	diva::vector<TargetStateEx> target_ex;
	ChanceState chance_time;
	diva::vector<TechZoneState> tech_zones;
	TechZoneDispState tz_disp;
	size_t tech_zone_index;
	UIState ui;
	int32_t effect_buffer[MaxHitEffectCount] = {};
	int32_t effect_index                     = 0;
	diva::optional<SongEntry> nc_song_entry;
	diva::optional<ChartEntry> nc_chart_entry;
	diva::map<int32_t, diva::string> fail_target_effect_map;
	diva::map<int32_t, diva::string> success_target_effect_map;
	ScoreState score;
};
#pragma pack(pop)

extern "C" {
typedef bool (*CheckSongHasStyleAvailable) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef SongEntry *(*FindSongEntry) (i32 pv);
typedef DifficultyEntry *(FindDifficultyEntry)(i32 pv, i32 difficulty, i32 edition);
typedef ChartEntry *(*FindChart) (i32 pv, i32 difficulty, i32 edition, i32 style);
typedef StateEx *(*GetState) ();
typedef bool (*DbReady) ();
}
} // namespace nc
