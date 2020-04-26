cbuffer cbCameraInfo : register(b0)
{
	matrix gmtxViewProjection : packoffset(c0);
};

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
VS_UI_OUTPUT VSUI(uint nVertexID : SV_VertexID, uint nInstanceID : SV_InstanceID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0) {
		output.position = float4(-1.0, 0.8, 0.0, 1.0);
		output.color = float4(0.0, 0.0, 1.0, 1.0);
	}
	else if (nVertexID == 1) {
		output.position = float4(-0.9, 1.0, 0.0, 1.0);
		output.color = float4(1.0, 0.0, 0.0, 1.0);
	}
	else if (nVertexID == 2) {
		output.position = float4(-0.8, 0.8, 0.0, 1.0);
		output.color = float4(0.0, 1.0, 0.0, 1.0);
	}

	output.position.x += float(0.2 * nInstanceID);

	return output;
}
float4 PSUI(VS_UI_OUTPUT input) : SV_TARGET
{
	return input.color;
}

struct VS_INSTANCING_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float4x4 mtxTransform : WORLDMATRIX;
	float4 instanceColor : INSTANCECOLOR;
};
struct VS_INSTANCING_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
{
	VS_INSTANCING_OUTPUT output;
	output.position = mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxViewProjection);
	output.color = input.color + input.instanceColor;
	return output; 
}
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	return input.color;
}