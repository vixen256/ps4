Texture2D<float4> tex : register(t0);
SamplerState Sampler : register(s0);

void ps(in float4 position : SV_Position, in float2 texCoord : TEXCOORD, out float4 color : SV_TARGET)
{
    float4 rgba = tex.SampleLevel(Sampler, texCoord, 0);
    color = float4(pow(rgba.xyz, float3(1.25, 1.25, 1.25)), rgba.w);
}
