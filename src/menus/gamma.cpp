#include "diva.h"
#include "gamma_ps.fxh"

using namespace diva;

namespace gamma {
bool inited = false;
p_dx_pixel_shader pixel_shader;

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

	set_ps_textures (rend_data_ctx, 0, 1, tex);
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
} // namespace gamma
