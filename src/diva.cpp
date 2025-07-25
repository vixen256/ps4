#include "diva.h"
#include "SigScan.h"

extern i32 theme;
namespace diva {
SIG_SCAN (sigOperatorNew, 0x1409777D0,
          "\x40\x53\x48\x83\xEC\x20\x48\x8B\xD9\xEB\x0F\x48\x8B\xCB\xE8\x00\x00\x00\x00\x85\xC0\x74\x13\x48\x8B\xCB\xE8\x00\x00\x00\x00\x48\x85\xC0\x74\xE7\x48\x83\xC4\x20\x5B\xC3\x48\x83\xFB\xFF\x74"
          "\x06\xE8\x00\x00\x00\x00\xCC\xE8\x00\x00\x00\x00\xCC\x40\x53",
          "xxxxxxxxxxxxxxx????xxxxxxxx????xxxxxxxxxxxxxxxxxx????xx????xxx");
SIG_SCAN (sigOperatorDelete, 0x1409B1E90,
          "\x48\x85\xC9\x74\x37\x53\x48\x83\xEC\x20\x4C\x8B\xC1\x33\xD2\x48\x8B\x0D\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x85\xC0\x75\x17\xE8\x00\x00\x00\x00\x48\x8B\xD8\xFF\x15\x00\x00\x00\x00\x8B"
          "\xC8\xE8\x00\x00\x00\x00\x89\x03\x48\x83\xC4\x20\x5B\xC3",
          "xxxxxxxxxxxxxxxxxx????xx????xxxxx????xxxxx????xxx????xxxxxxxx");

FUNCTION_PTR (void *, GetInputState, 0x1402AC970, i32 a1);
FUNCTION_PTR (bool, IsButtonTapped, 0x1402AB260, void *state, Button button);
FUNCTION_PTR (void *, CreateAetLayerData, 0x14028D560, AetLayerArgs *args, i32 aetSceneId, const char *layerName, i32 priority, AetAction action);
FUNCTION_PTR (i32, PlayAetLayer, 0x1402CA220, AetLayerArgs *args, i32 id);
FUNCTION_PTR (void, GetComposition, 0x1402CA670, AetComposition *composition, i32 id);
FUNCTION_PTR (void, ApplyAetLayerLocation, 0x14065FCC0, AetLayerArgs *args, Vec3 *locationData);
FUNCTION_PTR (void, PlaySoundEffect, 0x1405AA540, const char *name, float volume);
FUNCTION_PTR (u64, GetPvLoadData, 0x14040B2A0);
FUNCTION_PTR (i32, GetCurrentStyle, 0x1401D64F0);
FUNCTION_PTR (InputType, NormalizeInputType, 0x1402ACAA0, i32 inputType);
FUNCTION_PTR (void, FreeSubLayers, 0x1401AC240, AetComposition *sublayerData, AetComposition *sublayerData2, void *first_element);
FUNCTION_PTR (void, RealStopAet, 0x1402CA330, i32 *id);
FUNCTION_PTR (void *, operatorNew, sigOperatorNew (), u64);
FUNCTION_PTR (void *, operatorDelete, sigOperatorDelete (), void *);
FUNCTION_PTR (void, FreeString, 0x14014BCD0, string *);
FUNCTION_PTR (void, GetFSCTRankData, 0x1401E7C60, i32 *fsRank, i32 *ctRank, i32 *fsPoints, i32 *ctPoints);
FUNCTION_PTR (bool, IsSurvival, 0x14023B6A0);
FUNCTION_PTR (bool, SurvivalCleared, 0x14023B950);
FUNCTION_PTR (i32, LifeGauge, 0x14023B890);
FUNCTION_PTR (Vec2 *, UpdateKeyAnm, 0x14060A840, Vec2 *a1, UpdateKeyAnmData *a2);
FUNCTION_PTR (FontInfo *, GetFont, 0x1402C4DC0, FontInfo *font, FontId id);
FUNCTION_PTR (FontInfo *, GetLangFont, 0x1402C4E10, FontInfo *font, FontId id, bool langSpecific);
FUNCTION_PTR (void, SetFontSize, 0x1402C5240, FontInfo *font, f32 width, f32 height);
FUNCTION_PTR (DrawParams *, DefaultDrawParams, 0x1402C53D0, DrawParams *params);
FUNCTION_PTR (void, DrawTextA, 0x1402C57B0, DrawParams *params, u32 flags, const char *text);
FUNCTION_PTR (void, DefaultSprArgs, 0x1405B78D0, SprArgs *args);
FUNCTION_PTR (SprArgs *, DrawSpr, 0x1405B49C0, SprArgs *args);
FUNCTION_PTR (Texture *, TextureLoadTex2D, 0x1405F0720, u32 id, i32 format, u32 width, u32 height, i32 mip_levels, void **data, i32, bool generate_mips);

vector<PvDbEntry *> *pvs             = (vector<PvDbEntry *> *)0x141753818;
vector<AetDbSceneEntry> *aetDbScenes = (vector<AetDbSceneEntry> *)0x1414AB588;
map<i32, AetData> *aets              = (map<i32, AetData> *)0x1414AB448;

void
drawTextAtPlaceholder (AetLayoutData *placeholder, DrawParams *params, i32 flags, const char *text) {
	auto height = placeholder->height;
	if (placeholder->resolutionMode == RESOLUTION_MODE_HD) height /= 1.5;
	SetFontSize (params->font, params->font->size.x, height);

	auto position       = Vec2 (placeholder->position.x, placeholder->position.y);
	params->lineOrigin  = position;
	params->textCurrent = position;
	params->colour[3]   = placeholder->opacity * 0xFF;
	diva::DrawTextA (params, flags, text);
}

void
drawTextAtPlaceholderCenter (AetLayoutData *placeholder, DrawParams *params, const char *text) {
	auto height = placeholder->height;
	if (placeholder->resolutionMode == RESOLUTION_MODE_HD) height /= 1.5;
	SetFontSize (params->font, params->font->size.x, height);

	auto length    = placeholder->position.x - placeholder->width;
	auto fontWidth = params->font->size.x;
	if (strlen (text) >= length / fontWidth) SetFontSize (params->font, length * fontWidth / strlen (text), params->font->size.y);

	auto position       = Vec2 (placeholder->position.x + placeholder->width / 2.0, placeholder->position.y + placeholder->height / 2.0);
	params->lineOrigin  = position;
	params->textCurrent = position;
	params->colour[3]   = placeholder->opacity * 0xFF;
	diva::DrawTextA (params, 0x28, text);

	if (params->font->size.x != fontWidth) SetFontSize (params->font, fontWidth, params->font->size.y);
}

void
drawTextAtPlaceholderCenterShadow (AetLayoutData *placeholder, DrawParams *params, const char *text) {
	auto height = placeholder->height;
	if (placeholder->resolutionMode == RESOLUTION_MODE_HD) height /= 1.5;
	SetFontSize (params->font, params->font->size.x, height);

	auto length    = placeholder->position.x - placeholder->width;
	auto fontWidth = params->font->size.x;
	if (strlen (text) >= length / fontWidth) SetFontSize (params->font, length * fontWidth / strlen (text), params->font->size.y);

	auto position     = Vec2 (placeholder->position.x + placeholder->width / 2.0, placeholder->position.y + placeholder->height / 2.0);
	params->colour[3] = placeholder->opacity * 0xFF;
	u8 colour[4];
	memcpy (colour, params->colour, sizeof (params->colour));

	params->lineOrigin  = position + 2;
	params->textCurrent = position + 2;
	params->colour[0]   = 0;
	params->colour[1]   = 0;
	params->colour[2]   = 0;
	diva::DrawTextA (params, 0x28, text);

	params->lineOrigin  = position;
	params->textCurrent = position;
	memcpy (params->colour, colour, sizeof (params->colour));
	diva::DrawTextA (params, 0x28, text);

	if (params->font->size.x != fontWidth) SetFontSize (params->font, fontWidth, params->font->size.x);
}

FontInfo::FontInfo () {
	memset (this, 0, sizeof (FontInfo));
	GetLangFont (this, FontId::FNT_36_DIVA_36_38, true);
}

DrawParams::DrawParams () {
	memset (this, 0, sizeof (DrawParams));
	DefaultDrawParams (this);
}

SprArgs::SprArgs () { DefaultSprArgs (this); }

void
appendThemeInPlace (char *name) {
	switch (theme) {
	case 1: strcat (name, "_f"); break;
	case 2: strcat (name, "_t"); break;
	default: strcat (name, "_ft"); break;
	}
}

char *
appendTheme (const char *name) {
	char *themeStr = (char *)calloc (strlen (name) + 5, sizeof (char));
	strcpy (themeStr, name);
	appendThemeInPlace (themeStr);

	return themeStr;
}

void
appendStringInPlace (string *str, const char *append) {
	u64 lengthNeeded = str->length + strlen (append) + 1;
	if (lengthNeeded > str->capacity) {
		char *temp = (char *)calloc (lengthNeeded, sizeof (char));
		if (str->capacity > 15) {
			strcpy (temp, str->ptr);
			free (str->ptr);
		} else {
			strcpy (temp, str->data);
		}
		strcat (temp, append);
		str->ptr      = temp;
		str->capacity = lengthNeeded;
		str->length   = lengthNeeded;
	} else {
		strcat (str->data, append);
		str->length = lengthNeeded;
	}
}

void
appendThemeInPlaceString (string *name) {
	switch (theme) {
	case 1: appendStringInPlace (name, "_f"); break;
	case 2: appendStringInPlace (name, "_t"); break;
	default: appendStringInPlace (name, "_ft"); break;
	}
}

InputType
getInputType () {
	return NormalizeInputType (*(i32 *)((u64)GetInputState (0) + 0x2E8));
}

bool
isMovieOnly (PvDbEntry *entry) {
	if (!entry) return false;
	for (auto i = 0; i < 5; i++)
		for (auto it = entry->difficulties[i].begin (); it != entry->difficulties[i].end (); it++)
			if (it->isMovieOnly) return true;

	return false;
}

std::optional<PvDbEntry *>
getPvDbEntry (i32 id) {
	if (id < 0) return std::nullopt;
	for (auto entry = pvs->first; entry != pvs->last; entry++) {
		if ((*entry)->id != id) continue;
		return std::optional (*entry);
	}
	return std::nullopt;
}

Vec4
getPlaceholderRect (AetLayoutData layer) {
	float xDiff = layer.width / 2;
	float yDiff = layer.height / 2;

	Vec4 vec;
	vec.x = layer.position.x - xDiff;
	vec.y = layer.position.x + xDiff;
	vec.z = layer.position.y - yDiff;
	vec.w = layer.position.y + yDiff;

	return vec;
}

Vec2
getClickedPos (void *inputState) {
	Vec2 initialVec (*(i32 *)((u64)inputState + 0x158), *(i32 *)((u64)inputState + 0x15C));

	RECT rect;
	GetClientRect (FindWindow ("DIVAMIXP", 0), &rect);
	float x = initialVec.x / (float)rect.right * 1920;
	float y = initialVec.y / (float)rect.bottom * 1080;

	return Vec2 (x, y);
}

void
StopAet (i32 *id) {
	RealStopAet (id);
	if (id != 0) *id = 0;
}

AetLayerArgs::AetLayerArgs (const char *sceneName, const char *layerName, i32 priority, AetAction action) { this->create (sceneName, layerName, priority, action); }
AetLayerArgs::AetLayerArgs (i32 sceneId, const char *layerName, i32 priority, AetAction action) { this->create (sceneId, layerName, priority, action); }

void
AetLayerArgs::create (const char *sceneName, const char *layerName, i32 priority, AetAction action) {
	i32 sceneId = -1;
	for (auto *it = aetDbScenes->begin (); it != aetDbScenes->end (); it++) {
		if (strcmp (it->name.c_str (), sceneName) == 0) {
			sceneId = it->id;
			break;
		}
	}
	CreateAetLayerData (this, sceneId, layerName, priority, action);
}

void
AetLayerArgs::create (i32 sceneId, const char *layerName, i32 priority, AetAction action) {
	CreateAetLayerData (this, sceneId, layerName, priority, action);
}

void
AetLayerArgs::play (i32 *id) {
	*id = PlayAetLayer (this, *id);
}

void
AetLayerArgs::setPosition (Vec3 position) {
	ApplyAetLayerLocation (this, &position);
}

template <>
AetComposition::~map () {
	for (auto it = this->begin (); it != this->end (); it = it->next ()) {
		it->pair.key.~string ();
		deallocate (it);
	}

	deallocate (this->root);
}

template <>
stringRange::_stringRangeBase (const char *str) {
	stringRange (str, strlen (str));
}
template <>
wstringRange::_stringRangeBase (const wchar_t *str) {
	wstringRange (str, wcslen (str));
}

std::multimap<std::string, taskAddition> taskAdditions;
HOOK (void, RunTask, 0x1402C9AC0, Task *task) {
	if (task->state != TaskState::RUNNING) originalRunTask (task);

	auto it = taskAdditions.equal_range (task->name);
	std::optional<taskFunction> func;
	for (auto functions = it.first; functions != it.second; functions++) {
		auto funcs = functions->second;
		if (task->op == TaskOp::INIT && (func = funcs.init)) {
			if (func.value () ((u64)task)) return;
		} else if (task->op == TaskOp::LOOP && (func = funcs.loop)) {
			if (func.value () ((u64)task)) return;
		} else if (task->op == TaskOp::DESTROY && (func = funcs.destroy)) {
			if (func.value () ((u64)task)) return;
		}
	}

	auto op = task->op;
	originalRunTask (task);
	if (op == TaskOp::LOOP && task->nextState == TaskState::NONE) {
		for (auto functions = it.first; functions != it.second; functions++) {
			auto funcs = functions->second;
			if (auto func = funcs.destroy) func.value () ((u64)task);
		}
	}
}

HOOK (void, RunTaskDisp, 0x1402C9B70, Task *task) {
	if ((task->state == TaskState::RUNNING || task->state == TaskState::SUSPENDED) && task->op != TaskOp::INIT && task->op != TaskOp::DESTROY) {
		auto it = taskAdditions.equal_range (task->name);
		for (auto functions = it.first; functions != it.second; functions++) {
			if (auto func = functions->second.display) {
				if (func.value () ((u64)task)) return;
			}
		}
	}

	originalRunTaskDisp (task);
}

void
addTaskAddition (const char *name, taskAddition addition) {
	taskAdditions.insert (std::pair (name, addition));
}

void
init () {
	INSTALL_HOOK (RunTask);
	INSTALL_HOOK (RunTaskDisp);
}
} // namespace diva
