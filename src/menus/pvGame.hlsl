Texture2D<float4> input : register(t0);
RWTexture2D<float4> output : register(u0);
SamplerState Sampler : register(s0);

[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadId : SV_DispatchThreadID) {
	output[dispatchThreadId] = float4(input.SampleLevel(Sampler, float2(dispatchThreadId.x / 1920.0, dispatchThreadId.y / 1080.0), 0).xyz, 1.0);
}
