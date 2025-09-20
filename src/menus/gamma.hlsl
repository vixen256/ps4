Texture2D<float4> tex : register(t0);
SamplerState Sampler : register(s0);

void vs(in uint vertexId : SV_VertexID, out float4 position : SV_Position, out float2 texCoord : TEXCOORD)
{
    texCoord = float2((vertexId << 1) & 2, vertexId & 2);
    position = float4(texCoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}

void ps(in float4 position : SV_Position, in float2 texCoord : TEXCOORD, out float4 color : SV_TARGET)
{
    float4 rgba = tex.SampleLevel(Sampler, texCoord, 0);
    color = float4(pow(rgba.xyz, float3(1.25, 1.25, 1.25)), rgba.w);
}
