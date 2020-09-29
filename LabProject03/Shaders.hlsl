
cbuffer cbPlayerInfo : register(b0) {
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1) {
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gf3CameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix		gmtxGameObject : packoffset(c0);
	uint		gnMaterial : packoffset(c4);
};
//정점 셰이더의 입력을 위한 구조체를 선언한다. 
struct VS_INPUT {
	float3 position : POSITION;
	float4 color : COLOR;
};
//정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다. 
struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
//정점 셰이더를 정의한다. 
VS_OUTPUT VSDiffused(VS_INPUT input) {
	VS_OUTPUT output;
	//정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다. 
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView),
		gmtxProjection);
	output.color = input.color;
	return(output);
}
//픽셀 셰이더를 정의한다. 
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	//return float4(1, 0, 0, 1);
	return(input.color);
}
// ====================플레이어========================== 
VS_OUTPUT VSPlayer(VS_INPUT input) {
	VS_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

// ====================인스턴싱========================== 
struct VS_INSTANCING_INPUT {
	float3 position : POSITION;
	float4 color : COLOR;
	float4x4 mtxTransform : WORLDMATRIX;
	float4 instanceColor : INSTANCECOLOR;
};
struct VS_INSTANCING_OUTPUT {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input) {
	VS_INSTANCING_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform),
		gmtxView), gmtxProjection);
	output.color = input.color + input.instanceColor;
	return(output);
}
float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

// ====================인스턴싱2==========================
struct INSTANCEDGAMEOBJECTINFO {
	matrix	m_mtxGameObject;
	float4	m_cColor;
};

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gGameObjectInfos : register(t0);
VS_INSTANCING_OUTPUT VSInstancing2(VS_INPUT input, uint nInstanceID : SV_InstanceID) {
	VS_INSTANCING_OUTPUT output;
	output.position = mul(mul(mul(float4(input.position, 1.0f), gGameObjectInfos[nInstanceID].m_mtxGameObject),
		gmtxView), gmtxProjection);
	output.color = input.color + gGameObjectInfos[nInstanceID].m_cColor;
	return output;
}
// ==============================================

struct VS_INPUT_LIGHT {
	float3		position : POSITION;
	float3		normal : NORMAL;
	float2		uv : TEXCOORD0;
};
struct VS_OUTPUT_LIGHT {
	float4		positionH : SV_POSITION;
	float3		positionW : POSITION;
	float3		normal : NORMAL0;
	float3		normalW : NORMAL1;
	float2		uv : TEXCOORD0;
};
#include "Light.hlsl"

VS_OUTPUT_LIGHT VSLighting(VS_INPUT_LIGHT input)
{
	VS_OUTPUT_LIGHT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxGameObject).xyz;
	output.positionH = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;
	output.normal = input.normal;
	output.uv = input.uv;
	return(output);
}
float4 PSLighting(VS_OUTPUT_LIGHT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
	//	float3 normalW = normalize(input.normalW);
	//	float3 cNormal = normalW * 0.5f + 0.5f;
	//	float4 color = float4(cNormal, 0.0f);
	float3 normalW = normalize(input.normalW);
	float4 color = Lighting(input.positionW, normalW);
	return(color);
#endif
}