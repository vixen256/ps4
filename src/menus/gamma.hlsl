Texture2D<float4> tex : register (t0);
SamplerState Sampler : register (s0);

static const float Gamma = 1.1;
// Based on https://github.com/shadps4-emu/shadPS4/blob/main/src/video_core/host_shaders/post_process.frag
void
ps (in float4 position : SV_Position, in float2 texCoord : TEXCOORD, out float4 color : SV_TARGET) {
	float4 rgba = tex.SampleLevel (Sampler, texCoord, 0);
	float3 rgb  = rgba.xyz > 0.0031308 ? 1.055 * pow (rgba.xyz, 1.0 / (2.4 + 1.0 - Gamma)) - 0.055 : 12.92 * rgba.xyz / Gamma;
	color       = float4 (pow (rgb, pow (Gamma, 2.4)), rgba.w);
}
