#include "DefaultShader.hlsli"

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_Target
{
	//return float4(1.0f, 1.0f, 0.0f, 1.0f);;
	return txDiffuse.Sample(samLinear, input.Tex);
}
