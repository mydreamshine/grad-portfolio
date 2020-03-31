// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
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
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 uv : TEXCOORD;
};


PSInput VS(VSInput vin)
{
    PSInput vout;

    float4 PosW = float4(vin.position, 1.0f);
    float3 NormalW = vin.normal;
#ifdef SKINNED
    if (vin.boneids[0] >= 0)
    {
        float4x4 boneTransform;
        if (vin.boneids[0] >= 0) boneTransform  = vin.bone_weights[0] * gBoneTransform[vin.boneids[0]];
        if (vin.boneids[1] >= 0) boneTransform += vin.bone_weights[1] * gBoneTransform[vin.boneids[1]];
        if (vin.boneids[2] >= 0) boneTransform += vin.bone_weights[2] * gBoneTransform[vin.boneids[2]];
        if (vin.boneids[3] >= 0) boneTransform += vin.bone_weights[3] * gBoneTransform[vin.boneids[3]];
        PosW = mul(float4(vin.position, 1.0f), boneTransform);
        NormalW = mul(NormalW, (float3x3)boneTransform);
    }
#endif

    PosW = mul(PosW, gWorld);

    // Transform to world space.
    vout.PosW = PosW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(NormalW, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(PosW, gViewProj);
    
    // �ؽ��� ��ǥ�� ��� ������ �ش� Ư���� �����Ѵ�.
    // ��� ���� Ư������ ���� �ﰢ���� ���� �����ȴ�.
    float4 uv = mul(float4(vin.uv, 0.0f, 1.0f), gTexTransform);
    vout.uv = mul(uv, gMatTransform).xy;

    return vout;
}

float4 PS(PSInput pin) : SV_TARGET
{
    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.uv) * gDiffuseAlbedo;

    // ������ �����ϸ� ���� ���̰� �ƴϰ� �� �� �����Ƿ� �ٽ� ����ȭ�Ѵ�.
    pin.NormalW = normalize(pin.NormalW);

    // ����Ǵ� ������ �������� ����
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // ���� ������ �䳻 ���� �ֺ���
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // ���� ����
    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // ��������� diffuse �������� ���ĸ� �����´�.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
