// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

// Include common HLSL code.
#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD;
#ifdef SKINNED
    float4 bone_weights : BONE_WEIHTS;
    int4   boneids : BONE_IDS;
#endif
};
struct PSInput
{
    float4 PosH    : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float3 PosW    : POSITION1;
    float3 NormalW : NORMAL;
    float2 uv : TEXCOORD;
};

// Matrix multiply order for mesh transform:
// LocalM * AnimM(BoneM) * WorldM (* ViewM * ProjM)
PSInput VS(VSInput vin)
{
    PSInput vout;

    float4 PosW = float4(vin.position, 1.0f);
    float3 NormalW = vin.normal;

    PosW = mul(PosW, gLocal);
    NormalW = mul(NormalW, (float3x3)gLocal);

#ifdef SKINNED
    if (vin.boneids[0] >= 0)
    {
        float4x4 animTransform;
        if (vin.boneids[0] >= 0) animTransform = vin.bone_weights[0] * gBoneTransform[vin.boneids[0]];
        if (vin.boneids[1] >= 0) animTransform += vin.bone_weights[1] * gBoneTransform[vin.boneids[1]];
        if (vin.boneids[2] >= 0) animTransform += vin.bone_weights[2] * gBoneTransform[vin.boneids[2]];
        if (vin.boneids[3] >= 0) animTransform += vin.bone_weights[3] * gBoneTransform[vin.boneids[3]];
        PosW = mul(float4(vin.position, 1.0f), animTransform);
        NormalW = mul(NormalW, (float3x3)animTransform);
    }
#endif

    PosW = mul(PosW, gWorld);

    // Transform to world space.
    vout.PosW = PosW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(NormalW, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(PosW, gViewProj);
    
    // 텍스쳐 좌표를 출력 정점의 해당 특성에 설정한다.
    // 출력 정점 특성들은 이후 삼각형을 따라 보간된다.
    float4 uv = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(uv, gMatTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(PosW, gShadowTransform);

    return vout;
}

float4 PS(PSInput pin) : SV_TARGET
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.uv) * gDiffuseAlbedo;
    diffuseAlbedo.a = diffuseAlbedo.a * gTexAlpha;

#ifdef ALPHA_TEST
    // 텍스처 알파가 0.1보다 작으면 픽셀을 폐기한다.
    // 이 판정을 최대한 일찍 수행하는 것이 바람직하다. 그러면 폐기 시
    // 셰이더의 나머지 코드의 실행을 생략할 수 있으므로 효율적이다.
    clip(diffuseAlbedo.a - 0.01f);
#endif

    // 법선을 보간하면 단위 길이가 아니게 될 수 있으므로 다시 정규화한다.
    pin.NormalW = normalize(pin.NormalW);

    // 조명되는 점에서 눈으로의 벡터
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // 간접 조명을 흉내 내는 주변광
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);

    // 직접 조명
    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // 통상적으로 diffuse 재질에서 알파를 가져온다.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
