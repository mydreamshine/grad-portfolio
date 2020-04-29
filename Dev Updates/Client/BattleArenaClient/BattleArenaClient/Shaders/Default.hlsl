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
    
    // �ؽ��� ��ǥ�� ��� ������ �ش� Ư���� �����Ѵ�.
    // ��� ���� Ư������ ���� �ﰢ���� ���� �����ȴ�.
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
    // �ؽ�ó ���İ� 0.1���� ������ �ȼ��� ����Ѵ�.
    // �� ������ �ִ��� ���� �����ϴ� ���� �ٶ����ϴ�. �׷��� ��� ��
    // ���̴��� ������ �ڵ��� ������ ������ �� �����Ƿ� ȿ�����̴�.
    clip(diffuseAlbedo.a - 0.01f);
#endif

    // ������ �����ϸ� ���� ���̰� �ƴϰ� �� �� �����Ƿ� �ٽ� ����ȭ�Ѵ�.
    pin.NormalW = normalize(pin.NormalW);

    // ����Ǵ� ������ �������� ����
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // ���� ������ �䳻 ���� �ֺ���
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);

    // ���� ����
    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // ��������� diffuse �������� ���ĸ� �����´�.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}
