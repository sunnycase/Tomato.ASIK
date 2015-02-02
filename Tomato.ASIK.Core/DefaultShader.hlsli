struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

cbuffer cbNeverChanges : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
};