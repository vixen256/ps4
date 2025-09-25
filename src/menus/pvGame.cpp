#include "diva.h"
#include <png.h>

namespace pvGame {
using namespace diva;
i32 prcInfoId = 0;
InputType previousInputType;

char modDir[MAX_PATH];
ID3D11Device *device;
ID3D11DeviceContext *context;

bool pvGameActive       = false;
bool takenScreenshot    = false;
bool startedWriteThread = false;

bool initedTexture = false;
p_dx_texture screenshotTexture;
p_dx_render_target screenshotTextureRtv;

ID3D11Texture2D *screenshotStagingTexture;

bool screenshotLoadingScreen = true;
Texture *gameTexture         = nullptr;

FUNCTION_PTR (bool, isPractice, 0x1401E7B90);

bool
PVGameInit (u64 a1) {
	if (isPractice ()) {
		InputType input   = diva::getInputType ();
		previousInputType = input;
		char buf[128];
		sprintf (buf, "prc_info_%02d", (i32)input);
		diva::AetLayerArgs layer ("AET_NSWGAM_GAME_MAIN", buf, 0xD, AetAction::IN_LOOP);
		layer.play (&prcInfoId);
	} else {
		pvGameActive = true;
		if (*(u64 *)(*(u64 *)0x141149808 + 0x48) == 0) printf ("[ps4] Couldn't access screenshot hook\n");
	}
	return false;
}

bool
PVGameLoop (u64 a1) {
	InputType input = diva::getInputType ();
	if (isPractice () && input != previousInputType) {
		StopAet (&prcInfoId);
		previousInputType = input;
		char buf[128];
		sprintf (buf, "prc_info_%02d", (i32)input);
		diva::AetLayerArgs layer ("AET_NSWGAM_GAME_MAIN", buf, 0xD, AetAction::LOOP);
		layer.play (&prcInfoId);
	}
	return false;
}

bool
PVGameDestroy (u64 a1) {
	StopAet (&prcInfoId);
	pvGameActive = false;
	return false;
}

void
write_png () {
	D3D11_MAPPED_SUBRESOURCE map;
	context->CopyResource (screenshotStagingTexture, screenshotTexture.texture->texture);
	HRESULT hr = context->Map (screenshotStagingTexture, 0, D3D11_MAP_READ, 0, &map);
	if (FAILED (hr)) {
		printf ("[ps4] Map %lx\n", hr);
		return;
	}

	char path[MAX_PATH];
	sprintf (path, "%s\\screenshots", modDir);
	CreateDirectoryA (path, nullptr);

	SYSTEMTIME datetime;
	GetSystemTime (&datetime);
	sprintf (path, "%s\\screenshots\\%d-%d-%dT%02d_%02d_%02d_%03d.png", modDir, datetime.wYear, datetime.wMonth, datetime.wDay, datetime.wHour, datetime.wMinute, datetime.wSecond,
	         datetime.wMilliseconds);

	png_image png;
	memset (&png, 0, sizeof (png_image));
	png.version          = PNG_IMAGE_VERSION;
	png.width            = 1920;
	png.height           = 1080;
	png.format           = PNG_FORMAT_RGBA;
	png.opaque           = nullptr;
	png.flags            = 0;
	png.colormap_entries = 0;

	png_image_write_to_file (&png, path, 0, map.pData, map.RowPitch, nullptr);

	context->Unmap (screenshotStagingTexture, 0);

	takenScreenshot = false;
}

HOOK (void, PassSprite, 0x1404DAA00, void *rend_data_ctx, u64 rndr_ptr) {
	if (*(u64 *)(*(u64 *)0x141149808 + 0x48) != 0 && *(i32 *)(*(u64 *)(*(u64 *)0x141149808 + 0x48) + 0x08) >= 3 && !takenScreenshot) {
		auto rndr = *(void **)(rndr_ptr + 0x30);
		auto tex  = dx_swapchain_ptr_get_render_target_textures ();

		if (!initedTexture) {
			p_dx_texture_create (&screenshotTexture, 1920, 1080, DX_TEXTURE_FORMAT_R8G8B8A8_UNORM, 0, 1, false, false);
			pair screenshotTexturePair (screenshotTexture, 0);
			pair depthPair (p_dx_texture (nullptr), 0);
			p_dx_render_target_create (&screenshotTextureRtv, &screenshotTexturePair, 1, &depthPair);
		}

		Vec4 clear_color (0.0, 0.0, 0.0, 0.0);
		set_render_target (rend_data_ctx, &screenshotTextureRtv);
		clear_render_target_view (rend_data_ctx, &clear_color);

		rndr_draw_quad_copy (*(void **)((u64)rndr + 0x1940), rend_data_ctx, 0, 0, 1920, 1080, tex, 1.0, 1.0, 0.0, true);

		set_render_target (rend_data_ctx, (p_dx_render_target *)(*(u64 *)0x141148218 + 0x30));
		takenScreenshot    = true;
		startedWriteThread = false;
	}

	return originalPassSprite (rend_data_ctx, rndr_ptr);
}

HOOK (void, PlayLoadingBg, 0x140654280, u64 a1) {
	std::vector<std::string> files;
	char path[MAX_PATH];
	WIN32_FIND_DATAA file;
	sprintf (path, "%s\\screenshots\\*.png", modDir);

	HANDLE handle = FindFirstFileA (path, &file);
	if (handle == INVALID_HANDLE_VALUE) return originalPlayLoadingBg (a1);
	do {
		if ((file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) files.push_back (std::string (file.cFileName));
	} while (FindNextFileA (handle, &file) != 0);

	if (files.size () == 0) return originalPlayLoadingBg (a1);

	sprintf (path, "%s\\screenshots\\%s", modDir, files.at (rand () % files.size ()).c_str ());

	png_image png;
	memset (&png, 0, sizeof (png_image));
	png.version = PNG_IMAGE_VERSION;
	png.opaque  = nullptr;
	png_image_begin_read_from_file (&png, path);

	if (png.warning_or_error == 2 || png.warning_or_error == 3) {
		printf ("[ps4] Failed to load image %s: %s\n", path, png.message);
		return originalPlayLoadingBg (a1);
	}

	if (png.width != 1920 || png.height != 1080) return originalPlayLoadingBg (a1);
	png.format = PNG_FORMAT_RGBA;
	void *data = malloc (1920 * 1080 * 4);

	png_image_finish_read (&png, nullptr, data, -(1920 * 4), nullptr);
	if (png.warning_or_error == 2 || png.warning_or_error == 3) {
		printf ("[ps4] Failed to load image %s: %s\n", path, png.message);
		return originalPlayLoadingBg (a1);
	}

	if (gameTexture != nullptr) TextureRelease (gameTexture);
	gameTexture = TextureLoadTex2D (0xBADC0FEE, 6, 1920, 1080, 0, &data, 1920 * 1080 * 4, false);
	free (data);

	*(u32 *)(a1 + 0x7C) = 0xBADC0FEE;
}

HOOK (void, DisplayLoadingBg, 0x140655B40, u64 a1) {
	if (*(u32 *)(a1 + 0x7C) == 0xBADC0FEE) {
		SprArgs args;
		args.id                     = -1;
		args.layer                  = 0x17;
		args.texture                = gameTexture;
		args.resolution_mode_screen = RESOLUTION_MODE_FHD;
		args.resolution_mode_sprite = RESOLUTION_MODE_FHD;
		DrawSpr (&args);
	}
	return originalDisplayLoadingBg (a1);
}

void
init (bool screenshot_loading_screen) {
	taskAddition addition;
	addition.init    = PVGameInit;
	addition.loop    = PVGameLoop;
	addition.destroy = PVGameDestroy;
	addTaskAddition ("PVGAME", addition);

	GetCurrentDirectoryA (MAX_PATH, modDir);
	screenshotLoadingScreen = screenshot_loading_screen;
}

void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	device  = Device;
	context = DeviceContext;

	HRESULT hr;

	D3D11_TEXTURE2D_DESC desc;

	desc.Width              = 1920;
	desc.Height             = 1080;
	desc.MipLevels          = 1;
	desc.ArraySize          = 1;
	desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage              = D3D11_USAGE_STAGING;
	desc.BindFlags          = 0;
	desc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags          = 0;

	hr = device->CreateTexture2D (&desc, nullptr, &screenshotStagingTexture);
	if (FAILED (hr)) {
		printf ("[ps4] CreateTexture2D Failed %lx\n", hr);
		return;
	}

	INSTALL_HOOK (PassSprite);
	if (screenshotLoadingScreen) {
		INSTALL_HOOK (PlayLoadingBg);
		INSTALL_HOOK (DisplayLoadingBg);
	}
}

void
OnFrame (IDXGISwapChain *swapChain) {
	if (takenScreenshot && !startedWriteThread) {
		std::thread t (write_png);
		t.detach ();
		startedWriteThread = true;
	}
}
} // namespace pvGame
