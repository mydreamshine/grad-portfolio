#include "Common.hlsl"

struct VSInput
{
	float3 position    : POSITION;
	float2 uv    : TEXCOORD;
};

struct PSInput
{
	float4 PosH    : SV_POSITION;
	float2 uv    : TEXCOORD;
};

PSInput VS(VSInput vin)
{
    PSInput vout = (PSInput)0.0f;

	float4 PosW = float4(vin.position, 1.0f);

	/*PosW.x = PosW.x - gRenderTargetSize.x / 2;
	PosW.y = PosW.y - gRenderTargetSize.y / 2;*/
	PosW.x = PosW.x * gInvRenderTargetSize.x * 2;
	PosW.y = PosW.y * gInvRenderTargetSize.y * 2;

	vout.PosH = PosW;
	
	// Output vertex attributes for interpolation across triangle.
    float4 uv = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(uv, gMatTransform).xy;
	vout.uv.y = -vout.uv.y;
	
    return vout;
}

float4 PS(PSInput pin) : SV_TARGET
{
    return gDiffuseMap.Sample(gsamAnisotropicWrap, pin.uv);
}


