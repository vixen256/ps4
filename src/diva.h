#pragma once
namespace diva {
#pragma pack(push, 8)

struct Vec2 {
	f32 x;
	f32 y;

	Vec2 () {
		this->x = 0;
		this->y = 0;
	}

	Vec2 (f32 x, f32 y) {
		this->x = x;
		this->y = y;
	}

	Vec2 operator+ (Vec2 other) { return Vec2 (this->x + other.x, this->y + other.y); }
	Vec2 operator- (Vec2 other) { return Vec2 (this->x - other.x, this->y - other.y); }
	Vec2 operator* (Vec2 other) { return Vec2 (this->x * other.x, this->y * other.y); }
	Vec2 operator/ (Vec2 other) { return Vec2 (this->x / other.x, this->y / other.y); }
	Vec2 operator+ (f32 offset) { return Vec2 (this->x + offset, this->y + offset); }
	Vec2 operator* (f32 scale) { return Vec2 (this->x * scale, this->y * scale); }
	Vec2 operator/ (f32 scale) { return Vec2 (this->x / scale, this->y / scale); }
};

struct Vec3 {
	f32 x;
	f32 y;
	f32 z;

	Vec3 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vec3 (f32 x, f32 y, f32 z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vec3 operator+ (Vec3 other) { return Vec3 (this->x + other.x, this->y + other.y, this->z + other.z); }
	Vec3 operator- (Vec3 other) { return Vec3 (this->x - other.x, this->y - other.y, this->z - other.z); }
	Vec3 operator* (Vec3 other) { return Vec3 (this->x * other.x, this->y * other.y, this->z * other.z); }
	Vec3 operator/ (Vec3 other) { return Vec3 (this->x / other.x, this->y / other.y, this->z / other.z); }
	Vec3 operator* (f32 scale) { return Vec3 (this->x * scale, this->y * scale, this->z * scale); }
	Vec3 operator/ (f32 scale) { return Vec3 (this->x / scale, this->y / scale, this->z / scale); }
};

struct Vec4 {
	f32 x;
	f32 y;
	f32 z;
	f32 w;

	Vec4 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

	Vec4 (f32 x, f32 y, f32 z, f32 w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	bool contains (Vec2 location) { return location.x > this->x && location.x < this->y && location.y > this->z && location.y < this->w; }
	Vec4 operator* (f32 scale) { return Vec4 (this->x * scale, this->y * scale, this->z * scale, this->w * scale); }
};

FUNCTION_PTR_H (void *, operatorNew, u64);
FUNCTION_PTR_H (void *, operatorDelete, void *);
struct string;
template <typename T>
T *
allocate (u64 count) {
	return (T *)(operatorNew (count * sizeof (T)));
}

inline void
deallocate (void *p) {
	operatorDelete (p);
}

struct string {
	union {
		char data[16];
		char *ptr;
	};
	u64 length;
	u64 capacity;

	char *c_str () {
		if (this->capacity > 15) return this->ptr;
		else return this->data;
	}

	string () {}
	string (const char *cstr) {
		u64 len = strlen (cstr);
		if (len > 15) {
			u64 new_len = (len + 1) | 0xF;
			this->ptr   = allocate<char> (new_len);
			strcpy (this->ptr, cstr);
			this->length   = len;
			this->capacity = new_len;
		} else {
			strcpy_s (this->data, cstr);
			this->length   = len;
			this->capacity = 15;
		}
	}

	string (char *cstr) {
		u64 len = strlen (cstr);
		if (len > 15) {
			u64 new_len = (len + 1) | 0xF;
			this->ptr   = allocate<char> (new_len);
			strcpy (this->ptr, cstr);
			this->length   = len;
			this->capacity = len;
		} else {
			strcpy_s (this->data, cstr);
			this->length   = len;
			this->capacity = 15;
		}
	}

	~string () {
		if (this->capacity > 15) deallocate (this->ptr);
		this->ptr      = 0;
		this->length   = 0;
		this->capacity = 15;
	}

	void extend (size_t len) {
		size_t extension = this->length + 1 + len;
		if (extension <= this->capacity) return;

		size_t new_capacity = extension | 0x0F;
		auto old            = this->c_str ();
		auto new_ptr        = allocate<char> (new_capacity);
		strcpy (new_ptr, old);

		if (this->capacity > 15) deallocate (this->ptr);
		this->ptr      = new_ptr;
		this->capacity = new_capacity;
	}

	void operator= (const char *rhs) {
		auto len = strlen (rhs);
		if (this->capacity <= len) this->extend (len + 1);
		strcpy (this->c_str (), rhs);
	}
	bool operator== (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return false;
		return strcmp (this->c_str (), rhs.c_str ()) == 0;
	}
	bool operator== (char *rhs) {
		if (!this->c_str () || !rhs) return false;
		return strcmp (this->c_str (), rhs) == 0;
	}
	auto operator<=> (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return 1;
		return strcmp (this->c_str (), rhs.c_str ());
	}
	auto operator<=> (char *rhs) {
		if (!this->c_str () || !rhs) return 1;
		return strcmp (this->c_str (), rhs);
	}
};

template <typename T>
struct listElement {
	listElement<T> *next;
	listElement<T> *previous;
	T current;

	bool operator== (const listElement<T> &rhs) { return this->current == rhs->current; }
	bool operator== (const T &rhs) { return this->current == rhs; }
};

template <typename T>
struct list {
	listElement<T> *root;
	u64 length;

	listElement<T> *begin () { return this->length ? this->root->next : this->root; }
	listElement<T> *end () { return this->root; }
};

template <typename K, typename V>
struct pair {
	K key;
	V value;
};

template <typename K, typename V>
struct mapElement {
	mapElement<K, V> *left;
	mapElement<K, V> *parent;
	mapElement<K, V> *right;
	bool isBlack;
	bool isNull;
	pair<K, V> pair;

	mapElement<K, V> *next () {
		auto ptr = this;
		if (ptr->right->isNull) {
			while (!ptr->parent->isNull && ptr == ptr->parent->right)
				ptr = ptr->parent;
			ptr = ptr->parent;
		} else {
			ptr = ptr->right;
			while (!ptr->left->isNull)
				ptr = ptr->left;
		}
		return ptr;
	}

	K *key () { return &this->pair.key; }

	V *value () { return &this->pair.value; }
};

template <typename K, typename V>
struct map {
	mapElement<K, V> *root;
	u64 length;

	map () {
		this->root          = allocate<mapElement<K, V>> (1);
		this->root->left    = this->root;
		this->root->parent  = this->root;
		this->root->right   = this->root;
		this->root->isBlack = true;
		this->root->isNull  = true;
		this->length        = 0;
	}

	~map () {
		for (auto it = this->begin (); it != this->end (); it = it->next ())
			deallocate (it);
		deallocate (this->root);
	}

	std::optional<V *> find (K key) {
		auto ptr = this->root->parent;
		while (!ptr->isNull)
			if (key == ptr->pair.key) return std::optional (ptr->value ());
			else if (key > ptr->pair.key) ptr = ptr->right;
			else if (key < ptr->pair.key) ptr = ptr->left;
		return std::nullopt;
	}

	mapElement<K, V> *begin () { return this->length ? this->root->left : this->root; }
	mapElement<K, V> *end () { return this->root; }
};

template <typename T>
struct vector {
	T *first;
	T *last;
	void *capacity_end;

	vector () {
		this->first        = nullptr;
		this->last         = nullptr;
		this->capacity_end = nullptr;
	}

	vector (u64 n) {
		this->first        = allocate<T> (n);
		this->last         = this->first;
		this->capacity_end = (void *)((u64)this->first + (n * sizeof (T)));
	}

	~vector () { deallocate (this->first); }

	std::optional<T *> at (u64 index) {
		if (index >= this->length ()) return std::nullopt;
		return std::optional (&this->first[index]);
	}

	void push_back (const T &value) {
		if (this->first == nullptr) {
			this->first        = allocate<T> (16);
			this->last         = this->first;
			this->capacity_end = (void *)((u64)this->first + (16 * sizeof (T)));
			memset ((void *)this->last, 0, sizeof (T));
			*this->last = value;
			this->last++;
			return;
		}

		if (this->remaining_capcity () > 0) {
			memset ((void *)this->last, 0, sizeof (T));
			*this->last = value;
			this->last++;
			return;
		}
	}

	void clear () { this->last = this->first; }

	u64 length () { return ((u64)this->last - (u64)this->first) / sizeof (T); }
	u64 capacity () { return ((u64)this->capacity_end - (u64)this->first) / sizeof (T); }
	u64 remaining_capcity () { return this->capacity () - this->length (); }

	T *begin () { return this->first; }
	T *end () { return this->last; }

	void reserve (u64 n) {
		if (this->capacity () > n) return;

		T *new_first   = allocate<T> (n);
		u64 old_length = (u64)this->last - (u64)this->first;
		memcpy ((void *)new_first, (void *)this->first, old_length);
		deallocate (this->first);

		this->first        = new_first;
		this->last         = (T *)((u64)new_first + old_length);
		this->capacity_end = (void *)((u64)new_first + (n * sizeof (T)));
	}

	void operator= (const vector<T> &other) {
		this->clear ();
		for (auto it = other.first; it != other.last; it++)
			this->push_back (*it);
	}
};

template <typename T>
struct _stringRangeBase {
	T *data;
	T *end;

	// Technically incorrect but always seems to work
	T *c_str () { return data; }

	_stringRangeBase () {
		data = 0;
		end  = 0;
	}
	_stringRangeBase (const T *str);
	_stringRangeBase (size_t length) {
		if (length <= 0) length = 8;
		data = allocate<T> (length);
		end  = data + length;
		memset (data, 0, length);
	}
};
using stringRange  = _stringRangeBase<char>;
using wstringRange = _stringRangeBase<wchar_t>;

template <typename T>
struct optional {
	T value;
	bool hasValue;
};

template <typename T>
struct shared_ptr {
	T *ptr;
	u32 uses;
	u32 weaks;
};

template <typename T, u64 size>
struct array {
	T elems[size];
};

enum class State : i32 {
	STARTUP     = 0,
	ADVERTISE   = 1,
	GAME        = 2,
	DATA_TEST   = 3,
	TEST_MODE   = 4,
	APP_ERROR   = 5,
	CS_MENU     = 6,
	CUSTOMIZE   = 7,
	GALLERY     = 8,
	MENU_SWITCH = 9,
	GAME_SWITCH = 10,
	TSHIRT_EDIT = 11,
	MAX         = 12,
};

enum class SubState : i32 {
	DATA_INITIALIZE           = 0,
	SYSTEM_STARTUP            = 1,
	LOGO                      = 2,
	TITLE                     = 3,
	CONCEAL                   = 4,
	PV_SEL                    = 5,
	PLAYLIST_SEL              = 6,
	GAME                      = 7,
	DATA_TEST_MAIN            = 8,
	DATA_TEST_MISC            = 9,
	DATA_TEST_OBJ             = 10,
	DATA_TEST_STG             = 11,
	DATA_TEST_MOT             = 12,
	DATA_TEST_COLLISION       = 13,
	DATA_TEST_SPR             = 14,
	DATA_TEST_AET             = 15,
	DATA_TEST_AUTH3D          = 16,
	DATA_TEST_CHR             = 17,
	DATA_TEST_ITEM            = 18,
	DATA_TEST_PERF            = 19,
	DATA_TEST_PVSCRIPT        = 20,
	DATA_TEST_PRINT           = 21,
	DATA_TEST_CARD            = 22,
	DATA_TEST_OPD             = 23,
	DATA_TEST_SLIDER          = 24,
	DATA_TEST_GLITTER         = 25,
	DATA_TEST_GRAPHICS        = 26,
	DATA_TEST_COLLECTION_CARD = 27,
	DATA_TEST_PAD             = 28,
	TEST_MODE                 = 29,
	APP_ERROR                 = 30,
	UNK_31                    = 31,
	CS_MENU                   = 32,
	CS_COMMERCE               = 33,
	CS_OPTION_MENU            = 34,
	CS_UNK_TUTORIAL_35        = 35,
	CS_CUSTOMIZE_SEL          = 36,
	CS_UNK_TUTORIAL_37        = 37,
	CS_GALLERY                = 38,
	UNK_39                    = 39,
	UNK_40                    = 40,
	UNK_41                    = 41,
	MENU_SWITCH               = 42,
	UNK_43                    = 43,
	OPTION_MENU_SWITCH        = 44,
	UNK_45                    = 45,
	UNK_46                    = 46,
	MAX                       = 47,
};

enum class InputType : i32 {
	XBOX        = 0,
	PLAYSTATION = 1,
	SWITCH      = 2,
	STEAM       = 3,
	KEYBOARD    = 4,
	UNKNOWN     = 5,
};

enum class AetAction : i32 {
	NONE         = 0, // Ignore all markers, just play through it all
	IN_ONCE      = 1, // Start at st_in, end at ed_in
	IN_LOOP      = 2, // Start at st_in, play to ed_lp then jump to st_lp and keep looping
	LOOP         = 3, // Start at st_lp, end at ed_lp, loops
	OUT_ONCE     = 4, // Start at st_out, end at ed_out
	SPECIAL_ONCE = 5, // Start at st_sp, end to ed_lp
	SPECIAL_LOOP = 6, // Start at st_sp, play to ed_sp, then jump to st_lp and loop through ed_lp
	UNK          = 7, // Start at st_in, end at ed_in, probably loops?
};

enum class Button : i32 {
	UP       = 3,
	DOWN     = 4,
	LEFT     = 5,
	RIGHT    = 6,
	TRIANGLE = 7,
	SQUARE   = 8,
	BACK     = 9,
	ACCEPT   = 10,
	L1       = 11,
	R1       = 12,
	L2       = 13,
	R2       = 14,
	L3       = 15,
	R3       = 16,
};

struct AetLayerArgs {
	i32 sceneId;
	char *layerName;
	string StartMarker;
	string EndMarker;
	string LoopMarker;
	f32 startTime;
	f32 endTime;
	i32 flags;
	i32 unk_0x7C;
	i32 unk_0x80;
	i32 priority;
	i32 resolutionMode;
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Vec3 anchor;
	f32 frameSpeed;
	Vec4 color;
	map<string, i32> layerSprite;
	string sound_path;
	map<string, string> soundReplace;
	i32 soundQueueIndex;
	map<u32, u32> spriteReplace;
	map<u32, void *> spriteTexture;
	map<u32, u32> spriteDiscard;
	void *frameRateControl;
	bool soundVoice;
	i32 unk_0x154;
	i32 unk_0x158;
	i32 unk_0x15C;
	i32 id;
	i32 unk_0x164;
	Vec3 position_2;

	AetLayerArgs () {}
	AetLayerArgs (const char *sceneName, const char *layerName, i32 priority, AetAction action);
	AetLayerArgs (i32 sceneId, const char *layerName, i32 priority, AetAction action);
	void create (const char *sceneName, const char *layerName, i32 priority, AetAction action);
	void create (i32 sceneId, const char *layerName, i32 priority, AetAction action);
	void play (i32 *id);
	void setPosition (Vec3 position);
};

struct AetLayoutData {
	struct {
		Vec4 x;
		Vec4 y;
		Vec4 z;
		Vec4 w;
	} matrix;
	Vec3 position;
	Vec3 anchor;
	f32 width;
	f32 height;
	f32 opacity;
	u32 unk_64;
	i32 resolutionMode;
	u32 unk_6C;
	i32 unk_70;
	u8 blendMode;
	u8 transferFlags;
	u8 trackMatte;
	i32 unk_78;
	i32 unk_7C;
};

struct FCurve {
	u32 length;
	f32 *points;
};

struct AetLayerVideo {
	u8 blendMode;
	u8 flag;
	u8 matte;
	FCurve anchorX;
	FCurve anchorY;
	FCurve posX;
	FCurve posY;
	FCurve rotZ;
	FCurve scaleX;
	FCurve scaleY;
	FCurve opacity;
	void *_3d;
};

enum class AetItemType : u8 {
	AET_ITEM_TYPE_NONE        = 0,
	AET_ITEM_TYPE_VIDEO       = 1,
	AET_ITEM_TYPE_AUDIO       = 2,
	AET_ITEM_TYPE_COMPOSITION = 3,
};

struct AetVideoSrc {
	char *spriteName;
	i32 spriteIndex;
};

struct AetVideo {
	u8 color[3];
	u16 width;
	u16 height;
	f32 fpf;
	u32 sources_count;
	AetVideoSrc *sources;
};

struct AetAudio {
	u32 soundIndex;
};

struct AetLayer;
struct AetComp {
	u32 layersCount;
	AetLayer *layers;
};

union AetItem {
	void *none;
	AetVideo *video;
	AetAudio *audio;
	AetComp *comp;
};

struct AetLayer {
	char *name;
	f32 startTime;
	f32 endTime;
	f32 offsetTime;
	f32 timeScale;
	u16 flags;
	u8 quality;
	AetItemType itemType;
	AetItem item;
	u32 markerCount;
	void *markers;
	AetLayerVideo *video;
	u64 unk_0x40;
	u64 unk_0x48;
};

struct AetData {
	void *vftable;
	void *scene;
	AetComp *comp;
	AetLayer *layer;
	f32 startTime;
	f32 endTime;
	i32 flags;
	i32 unk_0x2C;
	i32 unk_0x30;
	f32 loopStart;
	f32 loopEnd;
	i32 unk_0x3C;
	i32 unk_0x40;
	i32 resolutionMode;
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Vec3 anchor;
	f32 frameSpeed;
	Vec4 color;
	f32 currentFrame;
};

template <typename T>
struct PvDbIndexedValue {
	i32 index;
	T value;
};

template <typename T>
struct PvDbIdValue {
	i32 index;
	i32 id;
	T value;
};

struct PvDbPlaceholder {};

struct PvDbExInfo {
	string key;
	string value;
};

struct PvDbDifficulty {
	i32 difficulty;
	i32 edition;
	i32 isExtra;
	// Theres something at +0x10 but I cannot figure it out and I don't need that info right now
	i32 unk[5];
	string scriptFile;
	i32 level; // Stars * 2;
	string buttonSoundEffect;
	string successSoundEffect;
	string slideSoundEffect;
	string slideChainStartSoundEffect;
	string slideChainSoundEffect;
	string slideChainSuccessSoundEffect;
	string slideChainFailureSoundEffect;
	string slideTouchSoundEffect;
	vector<PvDbIdValue<string>> motion[6];
	vector<PvDbPlaceholder> field;
	bool exStage;
	vector<PvDbIndexedValue<string>> items;
	vector<PvDbIdValue<string>> handItems;
	vector<PvDbIndexedValue<string>> editEffects;
	vector<PvDbPlaceholder> unk_240;
	vector<PvDbPlaceholder> unk_258;
	vector<PvDbPlaceholder> unk_270;
	f32 unk_288;
	f32 unk_28C;
	string unk_290;
	string music;
	string illustrator;
	string arranger;
	string manipulator;
	string editor;
	string guitar;
	PvDbExInfo exInfo[4];
	string unk_470;
	vector<PvDbIndexedValue<string>> movies;
	i32 movieSurface;
	bool isMovieOnly;
	string effectSoundEffect;
	vector<string> effectSoundEffectList;
	i32 version;
	i32 scriptFormat;
	i32 highSpeedRate;
	f32 hiddenTiming;
	f32 suddenTiming;
	bool editCharaScale;
	string unk_0x500;
	u64 unk_0x520;
	u64 unk_0x528;

	~PvDbDifficulty () = delete;
};

struct PvDbPerformer {
	i32 type;
	i32 chara;
	i32 costume;
	i32 pv_costume[5];
	bool fixed;
	i32 exclude;
	i32 size;
	i32 pseudo_same_id;
	i32 item[4];
	i32 unk;

	~PvDbPerformer () = delete;
};

struct PvDbEntry {
	i32 id;
	i32 date;
	string name;
	string nameReading;
	i32 unk_48;
	i32 bpm;
	string soundFile;
	vector<PvDbIndexedValue<string>> lyrics;
	f32 sabiStartTime;
	f32 sabiPlayTime;
	u64 unk_90;
	vector<PvDbPerformer> performers;
	vector<PvDbDifficulty> difficulties[5];
	INSERT_PADDING (0x288);
	i32 pack;

	bool HasDifficulty (i32 diff, bool extra) {
		for (auto it = this->difficulties[diff].begin (); it != this->difficulties[diff].end (); it++)
			if (it->isExtra == extra) return true;
		return false;
	}

	~PvDbEntry () = delete;
};

enum ModuleAttr : i32 {
	Swimsuit     = 1 << 0,
	NoSwap       = 1 << 1,
	FutureSound  = 1 << 2,
	ColorfulTone = 1 << 3,
	Tshirt       = 1 << 4,
};

struct ModuleData {
	i32 id;
	i32 sort_index;
	i32 chara;
	i32 cos;
	INSERT_PADDING (0x5C);
	u32 sprSetId;
	i32 unk_0x70;
	u32 spriteId;
	i32 unk_0x74;
	i32 unk_0x78;
	string name;
	i32 shop_price;
	u64 unk_0xA8;
	u64 unk_0xB0;
	ModuleAttr attr;
};

struct CustomizeItemData {
	i32 id;
	i32 obj_id;
	i32 sort_index;
	string name;
	i32 chara;
	i32 parts;
	INSERT_PADDING (0x24);
	i32 bind_module;
};

struct AetDbSceneEntry {
	stringRange name;
	i32 id;
};

class TaskInterface {
public:
	virtual ~TaskInterface () {}
	virtual bool Init ()    = 0;
	virtual bool Loop ()    = 0;
	virtual bool Destroy () = 0;
	virtual bool Display () = 0;
	virtual bool Basic ()   = 0;
};

enum class TaskOp : i32 {
	NONE = 0,
	INIT,
	LOOP,
	DESTROY,
	MAX,
};

enum class TaskState : i32 {
	NONE = 0,
	RUNNING,
	SUSPENDED,
	HIDDEN,
};

enum class TaskRequest : i32 {
	NONE = 0,
	INIT,
	DESTROY,
	SUSPEND,
	HIDE,
	RUN,
};

struct Task : public TaskInterface {
	i32 priority;
	Task *parent;
	TaskOp op;
	TaskState state;
	TaskRequest request;
	TaskOp nextOp;
	TaskState nextState;
	bool resetOnDestroy;
	bool isFrameDependent;
	char name[32];
};

typedef bool (*taskFunction) (u64 Task);
struct taskAddition {
	std::optional<taskFunction> init;
	std::optional<taskFunction> loop;
	std::optional<taskFunction> destroy;
	std::optional<taskFunction> display;
};

struct UpdateKeyAnmData {
	Vec3 position;
	Vec3 scale;
	f32 opacity;
	u32 layer;
	i32 kbBgL; // 59872
	i32 kbBgC; // 59871
	i32 kbBgR; // 59873
	Vec2 offset1;
	u32 xbSpriteId;
	u32 psSpriteId;
	u32 swSpriteId;
	u32 stSpriteId;
	vector<Button> keycodes;
	i32 unk60;
	Vec2 offset2;
	f32 width;

	UpdateKeyAnmData () {
		this->kbBgL    = 59872;
		this->kbBgC    = 59871;
		this->kbBgR    = 59873;
		this->unk60    = -1;
		this->opacity  = 1.0;
		this->scale.x  = 1.0;
		this->scale.y  = 1.0;
		this->scale.z  = 1.0;
		this->layer    = 0x1A;
		this->keycodes = vector<Button> (5);
	}
};

struct CoverSong {
	i32 vocalCharaNum;
	string name;
	string vocalDispName;
	string fileName;
};

enum resolutionMode : u32 {
	RESOLUTION_MODE_QVGA          = 0x00,
	RESOLUTION_MODE_VGA           = 0x01,
	RESOLUTION_MODE_SVGA          = 0x02,
	RESOLUTION_MODE_XGA           = 0x03,
	RESOLUTION_MODE_SXGA          = 0x04,
	RESOLUTION_MODE_SXGAPlus      = 0x05,
	RESOLUTION_MODE_UXGA          = 0x06,
	RESOLUTION_MODE_WVGA          = 0x07,
	RESOLUTION_MODE_WSVGA         = 0x08,
	RESOLUTION_MODE_WXGA          = 0x09,
	RESOLUTION_MODE_FWXGA         = 0x0A,
	RESOLUTION_MODE_WUXGA         = 0x0B,
	RESOLUTION_MODE_WQXGA         = 0x0C,
	RESOLUTION_MODE_HD            = 0x0D,
	RESOLUTION_MODE_FHD           = 0x0E,
	RESOLUTION_MODE_UHD           = 0x0F,
	RESOLUTION_MODE_3KatUHD       = 0x10,
	RESOLUTION_MODE_3K            = 0x11,
	RESOLUTION_MODE_QHD           = 0x12,
	RESOLUTION_MODE_WQVGA         = 0x13,
	RESOLUTION_MODE_qHD           = 0x14,
	RESOLUTION_MODE_XGAPlus       = 0x15,
	RESOLUTION_MODE_1176x664      = 0x16,
	RESOLUTION_MODE_1200x960      = 0x17,
	RESOLUTION_MODE_WXGA1280x900  = 0x18,
	RESOLUTION_MODE_SXGAMinus     = 0x19,
	RESOLUTION_MODE_FWXGA1366x768 = 0x1A,
	RESOLUTION_MODE_WXGAPlus      = 0x1B,
	RESOLUTION_MODE_HDPlus        = 0x1C,
	RESOLUTION_MODE_WSXGA         = 0x1D,
	RESOLUTION_MODE_WSXGAPlus     = 0x1E,
	RESOLUTION_MODE_1920x1440     = 0x1F,
	RESOLUTION_MODE_QWXGA         = 0x20,
	RESOLUTION_MODE_MAX           = 0x21,
};

enum class FontId {
	CMN_10X16 = 0,
	CMN_NUM12X16,
	CMN_NUM14X18,
	CMN_NUM14X20,
	CMN_NUM20X26,
	CMN_NUM20X22,
	CMN_NUM22X22,
	CMN_NUM22X24,
	CMN_NUM24X30,
	CMN_NUM26X24,
	CMN_NUM28X40,
	CMN_NUM28X40_GOLD,
	CMN_NUM34X32,
	CMN_NUM40X52,
	CMN_NUM56X46,
	CMN_ASC12X18,
	FNT_36_DIVA_36_38_24,
	BOLD36_DIVA_36_38_24,
	FNT_36_DIVA_36_38,
	BOLD36_DIVA_36_38,
	FNT_36CN_DIVA_36_38,
	BOLD36CN_DIVA_36_38,
	FNT_36CN2_DIVA_36_38,
	BOLD36CN2_DIVA_36_38,
	FNT_36KR_DIVA_36_38,
	BOLD36KR_DIVA_36_38,
	FNT_36LATIN9_DIVA_36_38_24,
	BOLD36LATIN9_DIVA_36_38_24,
	FNT_36LATIN9_DIVA_36_38,
	BOLD36LATIN9_DIVA_36_38,
};

struct FontInfo {
	FontId fontId;
	void *rawfont;
	i32 sprId;
	i32 unk_0x14;
	f32 unk_0x18;
	f32 unk_0x1C;
	f32 width;
	f32 height;
	f32 unk_0x28;
	f32 unk_0x2C;
	Vec2 size;
	f32 scaledWidth;
	f32 scaledHeight;
	f32 unk_0x40;
	f32 unk_0x44;

	FontInfo ();
};

struct DrawParams {
	f32 unk_0x00;
	u8 colour[4];
	u8 fillColour[4];
	bool clip;
	Vec4 clipData;
	u32 layer;
	u32 priority;
	resolutionMode resolutionMode;
	u32 unk_0x2C;
	Vec2 textCurrent;
	Vec2 lineOrigin;
	u64 lineLength;
	FontInfo *font;
	u16 unk_0x50;
	u32 unk_0x54;
	u32 unk_0x58;
	u32 unk_0x5C;
	u32 unk_0x60;

	DrawParams ();
};

enum DirectXTextureFormat : i32 {
	DX_TEXTURE_FORMAT_R8_UNORM           = 0x00,
	DX_TEXTURE_FORMAT_R8G8_UNORM         = 0x01,
	DX_TEXTURE_FORMAT_B5G6R5_UNORM       = 0x02,
	DX_TEXTURE_FORMAT_R8G8B8A8_UNORM     = 0x03,
	DX_TEXTURE_FORMAT_B8G8R8A8_UNORM     = 0x04,
	DX_TEXTURE_FORMAT_R32_FLOAT          = 0x05,
	DX_TEXTURE_FORMAT_R32G32_FLOAT       = 0x06,
	DX_TEXTURE_FORMAT_R16G16_FLOAT       = 0x07,
	DX_TEXTURE_FORMAT_R11G11B10_FLOAT    = 0x08,
	DX_TEXTURE_FORMAT_R16G16B16A16_FLOAT = 0x09,
	DX_TEXTURE_FORMAT_BC1_UNORM          = 0x0A,
	DX_TEXTURE_FORMAT_BC2_UNORM          = 0x0B,
	DX_TEXTURE_FORMAT_BC3_UNORM          = 0x0C,
	DX_TEXTURE_FORMAT_BC4_UNORM          = 0x0D,
	DX_TEXTURE_FORMAT_BC5_UNORM          = 0x0E,
	DX_TEXTURE_FORMAT_BC6H_UF16          = 0x0F,
	DX_TEXTURE_FORMAT_BC7_UNORM          = 0x10,
	DX_TEXTURE_FORMAT_R32_TYPELESS       = 0x11,
	DX_TEXTURE_FORMAT_B8G8R8X8_TYPELESS  = 0x12,
	DX_TEXTURE_FORMAT_UNKNOWN            = 0x13,
	DX_TEXTURE_FORMAT_MAX                = 0x14,
};

struct DirectXTexture {
	i32 ref_count;
	DirectXTexture *free_next;
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *resource_view;
	DirectXTextureFormat format;
	DirectXTextureFormat internal_format;
	i32 flags;
	i32 width;
	i32 height;
	i32 mip_levels;
};

struct Texture {
	i32 ref_count;
	i32 id;
	i32 flags;
	i16 width;
	i16 height;
	u32 target;
	u32 format;
	i32 max_mipmap;
	i32 size;
	i32 unk_0x20;
	DirectXTexture *dx_texture;
};

struct mat4 {
	Vec4 x;
	Vec4 y;
	Vec4 z;
	Vec4 w;
};

struct SprArgs {
	u32 kind;
	i32 id;
	u8 color[4];
	i32 attr;
	i32 blend;
	i32 index;
	i32 priority;
	i32 layer;
	resolutionMode resolution_mode_screen;
	resolutionMode resolution_mode_sprite;
	Vec3 center;
	Vec3 trans;
	Vec3 scale;
	Vec3 rot;
	Vec2 skew_angle;
	mat4 mat;
	Texture *texture;
	i32 shader;
	i32 field_AC;
	mat4 transform;
	bool field_F0;
	void *vertex_array;
	size_t num_vertex;
	i32 field_108;
	void *field_110;
	bool set_viewport;
	Vec4 viewport;
	u32 flags;
	Vec2 sprite_size;
	i32 field_138;
	Vec2 texture_pos;
	Vec2 texture_size;
	SprArgs *next;
	DirectXTexture *dx_texture;

	SprArgs ();
};

struct SaveDataModule {
	i32 unk;
	i32 moduleId;
	i32 accessory_head;
	i32 accessory_face;
	i32 accessory_chest;
	i32 accessory_back;
	i32 hair;
};

struct PvSpriteId {
	PvDbEntry *pvData;
	u32 setId;
	u32 bgId[4];
	u32 jkId[4];
	u32 logoId[4];
	u32 thumbId[4];
};

struct SurvivalSong {
	i32 id;
	i32 difficulty;
	i32 edition;
};

struct PvLoadInfo {
	i32 pvId;
	i32 version;
	i32 difficulty;
	i32 extra;
	i32 level;
	u8 unk_0x14;
	i32 pvId2;
	i32 unk_0x1C;
	u8 unk_0x20;
	u8 unk_0x21;
	i32 unk_0x24;
	u8 unk_0x28;
	i32 modifier;
	i32 modules[6];
	i32 unk_0x48[6];
	i32 accessories[6][5];
	bool accessories_enabled[6][5];
};
#pragma pack(pop)

extern vector<PvDbEntry *> *pvs;
extern map<i32, PvSpriteId> *pvSprites;
extern map<i32, AetData> *aets;
extern vector<ModuleData> *modules;
extern vector<string> *romDirs;

using AetComposition = map<string, AetLayoutData>;
template <>
AetComposition::~map ();

template <>
stringRange::_stringRangeBase (const char *str);
template <>
wstringRange::_stringRangeBase (const wchar_t *str);

FUNCTION_PTR_H (void *, GetInputState, i32 a1);
FUNCTION_PTR_H (bool, IsButtonTapped, void *state, Button button);
FUNCTION_PTR_H (bool, IsButtonDown, void *state, Button button);
FUNCTION_PTR_H (void, GetComposition, AetComposition *composition, i32 id);
FUNCTION_PTR_H (void, PlaySoundEffect, const char *name, f32 volume);
FUNCTION_PTR_H (u64, GetPvLoadData);
FUNCTION_PTR_H (i32, GetCurrentStyle);
FUNCTION_PTR_H (InputType, NormalizeInputType, i32 inputType);
FUNCTION_PTR_H (void, GetFSCTRankData, i32 *fsRank, i32 *ctRank, i32 *fsPoints, i32 *ctPoints);
FUNCTION_PTR_H (bool, IsSurvival);
FUNCTION_PTR_H (bool, SurvivalCleared);
FUNCTION_PTR_H (i32, LifeGauge);
FUNCTION_PTR_H (Vec2 *, UpdateKeyAnm, Vec2 *a1, UpdateKeyAnmData *a2);
FUNCTION_PTR_H (FontInfo *, GetFont, FontInfo *font, FontId id);
FUNCTION_PTR_H (FontInfo *, GetLangFont, FontInfo *font, FontId id, bool langSpecific);
FUNCTION_PTR_H (void, GetSpriteFont, FontInfo *font, i32 sprId, i32 width, i32 height);
FUNCTION_PTR_H (void, SetFontSize, FontInfo *font, f32 width, f32 height);
FUNCTION_PTR_H (void, DrawTextA, DrawParams *params, u32 flags, const char *text);
FUNCTION_PTR_H (void, DrawTextFmt, DrawParams *params, u32 flags, const char *fmt, ...);
FUNCTION_PTR_H (SprArgs *, DrawSpr, SprArgs *args);
FUNCTION_PTR_H (Texture *, TextureLoadTex2D, u32 id, i32 format, u32 width, u32 height, i32 mip_levels, void **data, i32, bool generate_mips);
FUNCTION_PTR_H (void, GetPlayerRank, i32 *exp, i32 *rank);
FUNCTION_PTR_H (void *, GetSaveData);
FUNCTION_PTR_H (void *, FindScore, void *saveData, i32 pvId);
FUNCTION_PTR_H (void *, GetScoreDifficulty, void *score, i32 unk, i32 diff, i32 extra);
FUNCTION_PTR_H (bool, IsScoreDifficultyUnlocked, void *scoreDifficulty);
FUNCTION_PTR_H (void *, FindModule, void *saveData, u32 moduleId);
FUNCTION_PTR_H (void *, FindCstmItem, void *saveData, u32 itemId);
FUNCTION_PTR_H (bool, CheckModuleUnlocked, void *moduleSaveData);
FUNCTION_PTR_H (void, LoadAetSet, i32 id, string *out);
FUNCTION_PTR_H (bool, LoadAetSetFinish, i32 id);
FUNCTION_PTR_H (void, LoadSprSet, i32 id, stringRange *out);
FUNCTION_PTR_H (bool, LoadSprSetFinish, i32 id);
FUNCTION_PTR_H (void, UnloadAetSet, i32 id);
FUNCTION_PTR_H (void, UnloadSprSet, i32 id);
FUNCTION_PTR_H (bool, ResolveFilePath, string *from, string *out);
FUNCTION_PTR_H (u32 *, getSpriteId, void *a1, stringRange *name);
FUNCTION_PTR_H (u32 *, getSprSetId, void *a1, stringRange *name);

void appendThemeInPlace (char *name);
char *appendTheme (const char *name);
void appendStringInPlace (string *str, const char *append);
void appendThemeInPlaceString (string *name);
InputType getInputType ();
bool isMovieOnly (PvDbEntry *entry);
std::optional<PvDbEntry *> getPvDbEntry (i32 id);
Vec4 getPlaceholderRect (AetLayoutData layer);
Vec2 getClickedPos (void *inputState);
void StopAet (i32 *id);
void addTaskAddition (const char *name, taskAddition addition);
void init ();
} // namespace diva
