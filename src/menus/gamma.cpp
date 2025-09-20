#include "gamma_ps.fxh"
#include "gamma_vs.fxh"

namespace gamma {
ID3D11Device *device;
ID3D11DeviceContext *context;
ID3D11VertexShader *vertex_shader          = nullptr;
ID3D11PixelShader *pixel_shader            = nullptr;
ID3D11SamplerState *sampler                = nullptr;
ID3D11RenderTargetView *render_target_view = nullptr;
ID3D11Texture2D *back_buffer_copy          = nullptr;
ID3D11ShaderResourceView *back_buffer_view = nullptr;

void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	device  = Device;
	context = DeviceContext;

	device->CreateVertexShader (vs_bytecode, sizeof (vs_bytecode), nullptr, &vertex_shader);
	device->CreatePixelShader (ps_bytecode, sizeof (ps_bytecode), nullptr, &pixel_shader);

	D3D11_SAMPLER_DESC sampler_desc = {};
	sampler_desc.Filter             = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	sampler_desc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.MipLODBias         = 0.0;
	sampler_desc.MaxAnisotropy      = 1;
	sampler_desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD             = 0.0;
	sampler_desc.MaxLOD             = 1.0;

	device->CreateSamplerState (&sampler_desc, &sampler);

	ID3D11Texture2D *back_buffer;
	SwapChain->GetBuffer (0, __uuidof (ID3D11Texture2D), (void **)&back_buffer);
	device->CreateRenderTargetView (back_buffer, nullptr, &render_target_view);

	D3D11_TEXTURE2D_DESC desc;
	back_buffer->GetDesc (&desc);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	device->CreateTexture2D (&desc, nullptr, &back_buffer_copy);
	device->CreateShaderResourceView (back_buffer_copy, nullptr, &back_buffer_view);

	back_buffer->Release ();
}

void
OnResize (IDXGISwapChain *SwapChain) {
	render_target_view->Release ();
	back_buffer_copy->Release ();
	back_buffer_view->Release ();

	ID3D11Texture2D *back_buffer;
	SwapChain->GetBuffer (0, __uuidof (ID3D11Texture2D), (void **)&back_buffer);
	device->CreateRenderTargetView (back_buffer, nullptr, &render_target_view);

	D3D11_TEXTURE2D_DESC desc;
	back_buffer->GetDesc (&desc);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	device->CreateTexture2D (&desc, nullptr, &back_buffer_copy);
	device->CreateShaderResourceView (back_buffer_copy, nullptr, &back_buffer_view);

	back_buffer->Release ();
}

void
OnFrame (IDXGISwapChain *SwapChain) {
	HRESULT hr;

	ID3D11Texture2D *back_buffer;
	hr = SwapChain->GetBuffer (0, __uuidof (ID3D11Texture2D), (void **)&back_buffer);
	if (FAILED (hr)) return;

	D3D11_VIEWPORT viewport = {};
	D3D11_TEXTURE2D_DESC desc;
	back_buffer->GetDesc (&desc);
	viewport.Width  = desc.Width;
	viewport.Height = desc.Height;

	context->CopyResource (back_buffer_copy, back_buffer);

	context->VSSetShader (vertex_shader, nullptr, 0);
	context->PSSetShader (pixel_shader, nullptr, 0);
	context->PSSetShaderResources (0, 1, &back_buffer_view);
	context->PSSetSamplers (0, 1, &sampler);
	context->OMSetRenderTargets (1, &render_target_view, nullptr);
	context->RSSetViewports (1, &viewport);
	context->Draw (6, 0);

	back_buffer->Release ();
}
} // namespace gamma
