#include "diva.h"
#include <png.h>

namespace pvGame {
using namespace diva;
i32 prcInfoId = 0;
InputType previousInputType;

char modDir[MAX_PATH];
ID3D11Device *device;
ID3D11DeviceContext *context;

bool pvGameActive                                 = false;
bool takenScreenshot                              = false;
ID3D11ComputeShader *shader                       = nullptr;
ID3D11Texture2D *screenshotTexture                = nullptr;
ID3D11Texture2D *screenshotStagingTexture         = nullptr;
ID3D11SamplerState *screenshotSampler             = nullptr;
ID3D11UnorderedAccessView *shaderReadWriteTexture = nullptr;

bool screenshotLoadingScreen = true;
ID3D11Texture2D *d3dTexture  = nullptr;
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

HOOK (void, DrawSprite, 0x1405B6400, void *a1, void *sprite_manager, SprArgs *args, void *matrix, i32 x_min, i32 y_min, i32 x_max, i32 y_max, void *a9, i32 ****render_ctx) {
	if (pvGameActive && *(u64 *)(*(u64 *)0x141149808 + 0x48) != 0 && *(i32 *)(*(u64 *)(*(u64 *)0x141149808 + 0x48) + 0x08) == 3 && args->layer > 3 && !takenScreenshot) {
		takenScreenshot   = true;
		auto instructions = **render_ctx;
		if (instructions[2] + 1 > instructions[1]) return;
		*instructions[2] = 0x90;
		instructions[2] += 1;
	}

	return originalDrawSprite (a1, sprite_manager, args, matrix, x_min, y_min, x_max, y_max, a9, render_ctx);
}

extern "C" {
void
write_png () {
	D3D11_MAPPED_SUBRESOURCE map;
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

	takenScreenshot = false;

	context->Unmap (screenshotStagingTexture, 0);
}

HOOK (void, ProcessRenderCommand, 0x1402B8526);
void
realProcessRenderCommand () {
	HRESULT hr;
	ID3D11RenderTargetView *render_target;
	ID3D11DepthStencilView *stencil_view;
	context->OMGetRenderTargets (1, &render_target, &stencil_view);

	D3D11_RENDER_TARGET_VIEW_DESC render_target_desc;
	render_target->GetDesc (&render_target_desc);
	if (render_target_desc.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2D) return;

	ID3D11Resource *render_resource;
	ID3D11Texture2D *render_texture;
	render_target->GetResource (&render_resource);
	hr = render_resource->QueryInterface (&render_texture);
	if (FAILED (hr)) {
		printf ("[ps4] QueryInterface %lx\n", hr);
		return;
	}

	ID3D11ShaderResourceView *shaderReadTexture = nullptr;
	hr                                          = device->CreateShaderResourceView (render_texture, nullptr, &shaderReadTexture);
	if (FAILED (hr)) {
		printf ("[ps4] CreateShaderResourceView %lx\n", hr);
		return;
	}

	context->OMSetRenderTargets (0, nullptr, nullptr);

	context->CSSetShader (shader, nullptr, 0);
	context->CSSetShaderResources (0, 1, &shaderReadTexture);
	context->CSSetUnorderedAccessViews (0, 1, &shaderReadWriteTexture, nullptr);
	context->CSSetSamplers (0, 1, &screenshotSampler);

	context->Dispatch (1920 / 8, 1080 / 8, 1);

	context->CopyResource (screenshotStagingTexture, screenshotTexture);

	std::thread t (write_png);
	t.detach ();

	render_texture->Release ();

	context->OMSetRenderTargets (1, &render_target, stencil_view);

	stencil_view->Release ();
	render_target->Release ();
}
}

HOOK (void, PlayLoadingBg, 0x140654280, u64 a1) {
	if (gameTexture == nullptr) gameTexture = TextureLoadTex2D (0xBADC0FEE, 6, 1920, 1080, 0, nullptr, 0, false);

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

	D3D11_MAPPED_SUBRESOURCE map;
	HRESULT hr = context->Map (d3dTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED (hr)) {
		printf ("[ps4] Map Failed %lx\n", hr);
		return originalPlayLoadingBg (a1);
	}

	for (int i = 0; i < 1080; i++)
		memcpy ((void *)((u64)map.pData + i * map.RowPitch), (void *)((u64)data + i * 1920 * 4), 1920 * 4);

	context->Unmap (d3dTexture, 0);
	free (data);

	context->CopyResource (gameTexture->dx_texture->texture, d3dTexture);

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
	desc.Usage              = D3D11_USAGE_DEFAULT;
	desc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags     = 0;
	desc.MiscFlags          = 0;
	hr                      = device->CreateTexture2D (&desc, nullptr, &screenshotTexture);
	if (FAILED (hr)) {
		printf ("[ps4] CreateTexture2D Failed %lx\n", hr);
		return;
	}

	desc.Usage          = D3D11_USAGE_STAGING;
	desc.BindFlags      = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr                  = device->CreateTexture2D (&desc, nullptr, &screenshotStagingTexture);
	if (FAILED (hr)) {
		printf ("[ps4] CreateTexture2D Failed %lx\n", hr);
		return;
	}

	if (screenshotLoadingScreen) {
		desc.Usage          = D3D11_USAGE_DYNAMIC;
		desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		hr                  = device->CreateTexture2D (&desc, nullptr, &d3dTexture);
		if (FAILED (hr)) {
			printf ("[ps4] CreateTexture2D Failed %lx\n", hr);
			return;
		}
	}

	hr = device->CreateUnorderedAccessView (screenshotTexture, nullptr, &shaderReadWriteTexture);
	if (FAILED (hr)) {
		printf ("[ps4] CreateUnorderedAccessView %lx\n", hr);
		return;
	}

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias     = 0.0;
	sampler_desc.MaxAnisotropy  = 1;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.BorderColor[0] = 0.0;
	sampler_desc.BorderColor[1] = 0.0;
	sampler_desc.BorderColor[2] = 0.0;
	sampler_desc.BorderColor[3] = 0.0;
	sampler_desc.MinLOD         = 0.0;
	sampler_desc.MaxLOD         = 0.0;
	hr                          = device->CreateSamplerState (&sampler_desc, &screenshotSampler);
	if (FAILED (hr)) {
		printf ("[ps4] CreateSamplerState %lx\n", hr);
		return;
	}

	ID3DBlob *shaderBlob;
	ID3DBlob *errorBlob;

	const char *shaderText = "\
Texture2D<float4> input : register(t0);\
RWTexture2D<float4> output : register(u0);\
SamplerState Sampler : register(s0);\
[numthreads(8, 8, 1)]\
void main(uint2 dispatchThreadId : SV_DispatchThreadID) {\
output[dispatchThreadId] = float4(input.SampleLevel(Sampler, float2(dispatchThreadId.x / 1920.0, dispatchThreadId.y / 1080.0), 0).xyz, 1.0);\
}";

	hr = D3DCompile (shaderText, strlen (shaderText), nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &shaderBlob, &errorBlob);
	if (FAILED (hr)) {
		MessageBoxA (0, (char *)errorBlob->GetBufferPointer (), "Shader compiler error", MB_OK | MB_ICONERROR);
		return;
	}
	device->CreateComputeShader (shaderBlob->GetBufferPointer (), shaderBlob->GetBufferSize (), nullptr, &shader);

	INSTALL_HOOK (DrawSprite);
	INSTALL_HOOK (ProcessRenderCommand);
	if (screenshotLoadingScreen) {
		INSTALL_HOOK (PlayLoadingBg);
		INSTALL_HOOK (DisplayLoadingBg);
	}
}
} // namespace pvGame
