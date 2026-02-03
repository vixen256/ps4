#include "diva.h"
#include "gamma_ps.fxh"

using namespace diva;

namespace gamma {
bool inited = false;
p_dx_pixel_shader pixel_shader;
dx_texture *in_texture = (dx_texture *)malloc (sizeof (dx_texture));
ID3D11Device *device;

HOOK (void, PassAdjustScreen, 0x1404a0f50, void *rndr, void *rend_data_ctx) {
	if (!inited) {
		p_dx_pixel_shader_create (&pixel_shader, ps_bytecode, sizeof (ps_bytecode));
		inited = true;
	}

	auto tex = dx_swapchain_ptr_get_render_target_textures ();

	Vec4 clear_color (0.0, 0.0, 0.0, 0.0);
	set_render_target (rend_data_ctx, (p_dx_render_target *)(*(u64 *)0x141148218 + 0x08));
	clear_render_target_view (rend_data_ctx, &clear_color);

	i32 x_offset = 0;
	i32 y_offset = 0;
	i32 width    = tex->texture->width;
	i32 height   = tex->texture->height;
	get_render_params (nullptr, &width, &height, &x_offset, &y_offset);
	set_viewport (rend_data_ctx, x_offset, y_offset, width, height);

	*in_texture = *tex->texture;
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels       = -1;
	device->CreateShaderResourceView (in_texture->texture, &desc, &in_texture->resource_view);

	p_dx_texture ptex = {in_texture};
	set_ps_textures (rend_data_ctx, 0, 1, &ptex);
	set_ps_sampler_state (rend_data_ctx, 0, 1, (void *)(*(u64 *)((u64)rndr + 0x1940) + 0x10));
	set_vs_shader (rend_data_ctx, (p_dx_vertex_shader *)(*(u64 *)((u64)rndr + 0x1940) + 0x130));
	set_ps_shader (rend_data_ctx, &pixel_shader);

	Vec4 color (1.0, 1.0, 1.0, 1.0);
	rndr_draw_quad (*(void **)((u64)rndr + 0x1940), rend_data_ctx, 0.0, width, height, 1.0, 1.0, 0.0, 0.0, 1.0, &color);

	return;
}

void
init () {
	INSTALL_HOOK (PassAdjustScreen);
}

void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	device = Device;
}
} // namespace gamma
