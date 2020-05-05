#include "Common.hlsl"

struct VSInput
{
	float3 position    : POSITION;
	float2 uv    : TEXCOORD;
#ifdef SKINNED
    float4 bone_weights : BONE_WEIHTS;
    int4   boneids  : BONE_IDS;
#endif
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

#ifdef SKINNED
    if (vin.boneids[0] >= 0)
    {
        float4x4 animTransform;
        if (vin.boneids[0] >= 0) animTransform = vin.bone_weights[0] * gBoneTransform[vin.boneids[0]];
        if (vin.boneids[1] >= 0) animTransform += vin.bone_weights[1] * gBoneTransform[vin.boneids[1]];
        if (vin.boneids[2] >= 0) animTransform += vin.bone_weights[2] * gBoneTransform[vin.boneids[2]];
        if (vin.boneids[3] >= 0) animTransform += vin.bone_weights[3] * gBoneTransform[vin.boneids[3]];
        PosW = mul(float4(vin.position, 1.0f), animTransform);
    }
#endif


    // Transform to world space.
    PosW = mul(PosW, gLocal);
    PosW = mul(PosW, gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(PosW, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
    float4 uv = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(uv, gMatTransform).xy;
	
    return vout;
}

// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void PS(PSInput pin)
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.uv) * gDiffuseAlbedo;

#ifdef ALPHA_TEST
    // 텍스처 알파가 0.1보다 작으면 픽셀을 폐기한다.
    // 이 판정을 최대한 일찍 수행하는 것이 바람직하다. 그러면 폐기 시
    // 셰이더의 나머지 코드의 실행을 생략할 수 있으므로 효율적이다.
    clip(diffuseAlbedo.a - 0.01f);
#endif
}


