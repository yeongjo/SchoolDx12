struct MATERIAL {
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular; //a = power
	float4					m_cEmissive;
};

cbuffer cbCameraInfo : register(b1) {
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix		gmtxGameObject : packoffset(c0);
	MATERIAL	gMaterial : packoffset(c4);
	uint		gnTexturesMask : packoffset(c8);
};

cbuffer cbFrameworkInfo : register(b3) {
	float 		gfCurrentTime;
	float		gfElapsedTime;
	float2		gf2CursorPos;
	float2		gfScreen;
};

cbuffer cbWaterInfo : register(b4) {
	matrix		gf4x4TextureAnimation : packoffset(c0);
};

cbuffer cbParticleInfo : register(b5) {
	float2		gf2SheetCount : packoffset(c0);
	float		gfStartTime : packoffset(c0.z);
	float		gfSheetSpeed : packoffset(c0.w);
};

/////////////////////////////////////////////////////////////////////
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
//#define _WITH_VERTEX_LIGHTING

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

#define _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
Texture2D gtxtAlbedoTexture : register(t6);
Texture2D gtxtSpecularTexture : register(t7);
Texture2D gtxtNormalTexture : register(t8);
Texture2D gtxtMetallicTexture : register(t9);
Texture2D gtxtEmissionTexture : register(t10);
Texture2D gtxtDetailAlbedoTexture : register(t11);
Texture2D gtxtDetailNormalTexture : register(t12);
#else
Texture2D gtxtStandardTextures[7] : register(t6);
#endif

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);
TextureCube gtxtSkyCubeTexture : register(t13);
#include "Light.hlsl"

struct VS_STANDARD_INPUT {
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_STANDARD_OUTPUT {
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float3 uv : TEXCOORD;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input) {
	VS_STANDARD_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.tangentW = (float3)mul(float4(input.tangent, 1.0f), gmtxGameObject);
	output.bitangentW = (float3)mul(float4(input.bitangent, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv.xy = input.uv;
	output.uv.z = output.position.z * 0.0010;

	return(output);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
	//return float4(1, 1, 0, 1);

	float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtAlbedoTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtSpecularTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtNormalTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtMetallicTexture.Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtEmissionTexture.Sample(gssWrap, input.uv);
#else
	if (gnTexturesMask & MATERIAL_ALBEDO_MAP) cAlbedoColor = gtxtStandardTextures[0].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_SPECULAR_MAP) cSpecularColor = gtxtStandardTextures[1].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) cNormalColor = gtxtStandardTextures[2].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_METALLIC_MAP) cMetallicColor = gtxtStandardTextures[3].Sample(gssWrap, input.uv);
	if (gnTexturesMask & MATERIAL_EMISSION_MAP) cEmissionColor = gtxtStandardTextures[4].Sample(gssWrap, input.uv);
#endif

	float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
	float3 normalW = input.normalW;
	if (gnTexturesMask & MATERIAL_NORMAL_MAP) {
		float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.bitangentW), normalize(input.normalW));
		float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] → [-1, 1]
		normalW = normalize(mul(vNormal, TBN));
		
	}
	float4 cIllumination = Lighting(input.positionW, normalW);
	cColor = cColor * cIllumination;

	half fogDensity = saturate(-0.2+0.8*exp2(-input.uv.z * input.uv.z));
	half3 fogColor = gtxtSkyCubeTexture.SampleLevel(gssClamp, input.positionW - gvCameraPosition, fogDensity * 5+6);
	cColor.rgb = lerp(fogColor, cColor, fogDensity);

	return(cColor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

struct VS_TEXTURED_INPUT {
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input) {
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	//return float4(1, 0, 1, 1);
	//return float4(input.uv, 0, 1);
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.uv);

	return(cColor);
}

VS_TEXTURED_OUTPUT VSViewport(VS_TEXTURED_INPUT input) {
	VS_TEXTURED_OUTPUT output;

	output.position = float4(input.position, 1.0f);
	output.position.z = 0;
	output.uv = input.uv;

	return(output);
}

#define SHARPEN_FACTOR 0.1

half4 sharpenMask(half2 fragCoord) {
	half4 up = gtxtTexture.Sample(gSamplerState, (fragCoord + half2(0, 1)/ gfScreen));
	half4 left = gtxtTexture.Sample(gSamplerState, (fragCoord + half2(-1, 0) / gfScreen));
	half4 center = gtxtTexture.Sample(gSamplerState, fragCoord);
	half4 right = gtxtTexture.Sample(gSamplerState, (fragCoord + half2(1, 0) / gfScreen));
	half4 down = gtxtTexture.Sample(gSamplerState, (fragCoord + half2(0, -1) / gfScreen));

	return (1.0 + 4.0 * SHARPEN_FACTOR) * center - SHARPEN_FACTOR * (up + left + right + down);
}

float4 PSViewport(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = sharpenMask(input.uv);
	cColor.rgb = gvCameraPosition.y < 50 ? cColor.rgb * float3(0.25, 0.54, 1) : cColor.rgb;
	return(cColor);
}
/////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SKYBOX_CUBEMAP_INPUT {
	float3 position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT {
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBox(VS_SKYBOX_CUBEMAP_INPUT input) {
	VS_SKYBOX_CUBEMAP_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.positionL = input.position;
	output.position.z = output.position.w;
	return(output);
}



float4 PSSkyBox(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyCubeTexture.Sample(gssClamp, input.positionL);
	//cColor = gtxtSkyCubeTexture.SampleLevel(gssClamp, input.positionL, 8);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_SPRITE_TEXTURED_INPUT {
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_SPRITE_TEXTURED_OUTPUT {
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SPRITE_TEXTURED_OUTPUT VSTextured(VS_SPRITE_TEXTURED_INPUT input) {
	VS_SPRITE_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

/*
float4 PSTextured(VS_SPRITE_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor;
	if (nPrimitiveID < 2)
		cColor = gtxtTextures[0].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 4)
		cColor = gtxtTextures[1].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 6)
		cColor = gtxtTextures[2].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 8)
		cColor = gtxtTextures[3].Sample(gWrapSamplerState, input.uv);
	else if (nPrimitiveID < 10)
		cColor = gtxtTextures[4].Sample(gWrapSamplerState, input.uv);
	else
		cColor = gtxtTextures[5].Sample(gWrapSamplerState, input.uv);
	float4 cColor = gtxtTextures[NonUniformResourceIndex(nPrimitiveID/2)].Sample(gWrapSamplerState, input.uv);

	return(cColor);
}
*/
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
	float2		uv : TEXCOORD;
};
struct VS_OUTPUT_LIGHT {
	float4		positionH : SV_POSITION;
	float3		positionW : POSITION;
	float3		normal : NORMAL0;
	float3		normalW : NORMAL1;
	float2		uv : TEXCOORD;
};

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
	return float4(1, 0, 0, 1);
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D<float4> gtxtTerrainBaseTexture : register(t14);
Texture2D<float4> gtxtTerrainDetailTextures[3] : register(t15); //t15, t16, t17
Texture2D<float4> gtxtTerrainAlphaTexture : register(t18);

struct VS_TERRAIN_INPUT {
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float3 normal : NORMAL;
};

struct VS_TERRAIN_OUTPUT {
	float4 position : SV_POSITION;
	float4 positionW : POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
	float3 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input) {
	VS_TERRAIN_OUTPUT output;
	float3 pos = input.position;
	float4 alpha = gtxtTerrainAlphaTexture.SampleLevel(gSamplerState, input.uv0, 0);
	pos.y *= alpha.w;
	output.positionW = mul(float4(pos, 1.0f), gmtxGameObject);
	output.position = mul(mul(output.positionW, gmtxView), gmtxProjection);
	output.color = input.color;
	output.uv0.xy = input.uv0;
	output.uv0.z = output.position.z * 0.0010;
	output.uv1 = input.uv1;
	output.normalW = mul(float4(input.normal, 0.0f), gmtxGameObject).xyz;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gSamplerState, input.uv0.xy);
	//	float fAlpha = gtxtTerrainAlphaTexture.Sample(gSamplerState, input.uv0);
	float fAlpha = gtxtTerrainAlphaTexture.Sample(gSamplerState, input.uv0.xy).w;
	//return float4(fAlpha, fAlpha, fAlpha, 1);
	float4 cDetailTexColors[3];
	cDetailTexColors[0] = gtxtTerrainDetailTextures[0].Sample(gSamplerState, input.uv1 * 2.0f);
	float2 off = float2(gfCurrentTime * 0.425f * 1.31f, gfCurrentTime * 0.021f);
	cDetailTexColors[1] = gtxtTerrainDetailTextures[1].Sample(gSamplerState, input.uv1 * 0.525f+ off); // 물
	cDetailTexColors[2] = gtxtTerrainDetailTextures[2].Sample(gSamplerState, input.uv1);

	float4 cColor = cBaseTexColor + (cDetailTexColors[0]-0.5f)*0.7f;
	cColor = lerp(cColor, cDetailTexColors[1]*0.5f, 1.0f - fAlpha);
	/*
		cColor = lerp(cDetailTexColors[0], cDetailTexColors[2], 1.0f - fAlpha) ;
		cColor = lerp(cBaseTexColor, cColor, 0.3f) + cDetailTexColors[1] * (1.0f - fAlpha);
	*/
	/*
		if (fAlpha < 0.35f) cColor = cDetailTexColors[2];
		else if (fAlpha > 0.8975f) cColor = cDetailTexColors[0];
		else cColor = cDetailTexColors[1];
	*/
	float3 normalW = normalize(input.normalW);
	//return float4(normalW, 1);
	cColor *= Lighting(input.positionW, normalW);
	half fogDensity = saturate(-0.2 + 0.8 * exp2(-input.uv0.z * input.uv0.z));
	half3 fogColor = gtxtSkyCubeTexture.SampleLevel(gssClamp, input.positionW - gvCameraPosition, fogDensity*8+3);
	cColor.rgb = lerp(fogColor, cColor, fogDensity);
	return(cColor);
}

struct VS_WATER_INPUT {
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
};
struct HullInputType {
	float3 position : POSITION;
	float3 uv : TEXCOORD0;
};
struct ConstantOutputType {
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};
struct HullOutputType {
	float3 position : POSITION;
	float2 uv : TEXCOORD0;
};
struct PixelInputType {
	float4 position : SV_POSITION;
	float3 uv : TEXCOORD0;
	float3 positionW : TEXCOORD1;
	float3 normal : NORMAL;
};

Texture2D<float4> gtxtWaterBaseTexture : register(t19);
Texture2D<float4> gtxtWaterDetail0Texture : register(t20);
Texture2D<float4> gtxtWaterDetail1Texture : register(t21);

cbuffer TessellationBuffer {
	float tessellationAmount;
	float3 padding;
};

HullInputType VSTerrainWater(VS_WATER_INPUT input) {
	HullInputType output;
	float3 pos = input.position;
	output.position = pos;
	output.uv.xy = input.uv;
	float3 camPos = gvCameraPosition;
	output.uv.z = length(camPos-mul(float4(pos,1), gmtxGameObject).xyz);
	return(output);
}

ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID) {
	ConstantOutputType output;
	float t = tessellationAmount;
	//t = 1;
	output.edges[0] = lerp(t, 1, saturate(pow(inputPatch[0].uv.z*0.001, 1)));
	output.edges[1] = lerp(t, 1, saturate(pow(inputPatch[1].uv.z*0.001, 1)));
	output.edges[2] = lerp(t, 1, saturate(pow(inputPatch[2].uv.z*0.001, 1)));
	output.inside = (output.edges[0]+ output.edges[1]+ output.edges[0])*0.3;
	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HullOutputType HSTerrainWater(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	HullOutputType output;
	output.position = patch[pointId].position;
	output.uv = patch[pointId].uv.xy;

	return output;
}

[domain("tri")]
PixelInputType DSTerrainWater(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch) {
	float3 vertexPosition;
	PixelInputType output;
	vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

	output.positionW = output.position = mul(float4(vertexPosition, 1.0f), gmtxGameObject);
	output.position = mul(output.position, gmtxView);
	output.position = mul(output.position, gmtxProjection);

	float heights[3] = {0,0,0};
	float2 uv1 = uvwCoord.x * patch[0].uv + uvwCoord.y * patch[1].uv + uvwCoord.z * patch[2].uv;;
	float2 uvOffset[3] = { float2(0.01, 0), float2(0, 0.01), float2(0, 0) };
	for (int i = 0;i <3;i++) {
		float2 uv = uvwCoord.x * (patch[0].uv+ uvOffset[i]) + uvwCoord.y * (patch[1].uv + uvOffset[i]) + uvwCoord.z * (patch[2].uv + uvOffset[i]);

		float4 cBaseTexColor = gtxtWaterBaseTexture.SampleLevel(gSamplerState, uv, 0);
		uv.y -= gfCurrentTime * 0.0425f * 1.31f;
		uv.x -= gfCurrentTime * 0.021f;
		float4 cBaseTexColor1 = gtxtWaterBaseTexture.SampleLevel(gSamplerState, uv, 0);
		uv.x += gfCurrentTime * 0.021f * 2;
		float4 cDetail0TexColor = gtxtWaterDetail0Texture.SampleLevel(gSamplerState, uv * 3.0f, 0);
		heights[i] = cBaseTexColor1 * cBaseTexColor * cDetail0TexColor * 100;
	}
	output.position.y += heights[2];

	output.normal.y = 35;
	output.normal.x = heights[0]-heights[2];
	output.normal.z = heights[1] - heights[2];
	//output.normal = mul(gmtxGameObject, normalize(output.normal));
	output.normal = normalize(output.normal);
	
	output.uv.xy = uv1;

	return output;
}

[maxvertexcount(16 * 4)]
void GSTerrainWater(triangle PixelInputType input[3], inout TriangleStream<PixelInputType> outStream) {
	PixelInputType output;
	for (int i = 0; i < 3; i++) {
		output.position = input[i].position;
		output.positionW = input[i].positionW;
		output.uv.xy = input[i].uv;
		output.uv.z = input[i].position.z * 0.0010;
		output.normal = input[i].normal;
		outStream.Append(output);
	}
	/*return;
	float t = 0.5;
	float4 i01 = lerp(input[0].position, input[1].position, t);
	float4 i02 = lerp(input[0].position, input[2].position, t);
	float4 i21 = lerp(input[2].position, input[1].position, t);
	float2 uv01 = lerp(input[0].uv, input[1].uv, t);
	float2 uv02 = lerp(input[0].uv, input[2].uv, t);
	float2 uv21 = lerp(input[2].uv, input[1].uv, t);

	output.position = input[1].position;
	output.uv = input[1].uv;
	outStream.Append(output);
	output.position = input[2].position;
	output.uv = input[2].uv;
	outStream.Append(output);
	output.position = i02;
	output.uv.xy = uv02;
	outStream.Append(output);
	output.position = input[0].position;
	output.uv = input[0].uv;
	outStream.Append(output);*/
	
	/*output.position = input[0].position;
	output.uv = input[0].uv;
	outStream.Append(output);
	output.position = i01;
	output.uv = uv01;
	outStream.Append(output);
	output.position = i02;
	output.uv = uv02;
	outStream.Append(output);
	
	output.position = input[1].position;
	output.uv = input[1].uv;
	outStream.Append(output);
	output.position = i21;
	output.uv = uv21;
	outStream.Append(output);
	output.position = i01;
	output.uv = uv01;
	outStream.Append(output);
	
	output.position = input[2].position;
	output.uv = input[2].uv;
	outStream.Append(output);
	output.position = i02;
	output.uv = uv02;
	outStream.Append(output);
	output.position = i01;
	output.uv = uv01;
	outStream.Append(output);

	output.position = i01;
	output.uv = uv01;
	outStream.Append(output);
	output.position = i21;
	output.uv = uv21;
	outStream.Append(output);
	output.position = i02;
	output.uv = uv02;
	outStream.Append(output);*/
	
	/*return;
	float sub_divisions = 2;
	float3 v0 = input[0].position;
	float3 v1 = input[1].position;
	float3 v2 = input[2].position;
	float dx = abs(v0.x - v2.x) / sub_divisions;
	float dz = abs(v0.z - v1.z) / sub_divisions;
	float x = v0.x;
	float z = v0.z;
	float3 uvv0 = input[0].position;
	float3 uvv1 = input[1].position;
	float3 uvv2 = input[2].position;
	float uvdx = abs(uvv0.x - uvv2.x) / sub_divisions;
	float uvdz = abs(uvv0.z - uvv1.z) / sub_divisions;
	float uvx = uvv0.x;
	float uvz = uvv0.z;
	for (int j = 0; j < sub_divisions*sub_divisions; j++) {
		output.position = float4(x, 0, z, 1);
		output.uv = float2(uvx, uvz);
		outStream.Append(output);
		output.position = float4(x, 0, z + dz, 1);
		output.uv = float2(uvx, uvz + uvdz);
		outStream.Append(output);
		output.position = float4(x + dx, 0, z, 1);
		output.uv = float2(uvx + uvdx, uvz);
		outStream.Append(output);
		output.position = float4(x + dx, 0, z + dz, 1);
		output.uv = float2(uvx + uvdx, uvz + uvdz);
		outStream.Append(output);

		x += dx;
		if ((j + 1) % sub_divisions == 0) {
			x = v0.x;
			z += dz;
		}
	}*/
}

half3 BoxProjection_half(half3 direction, half3 position) {
	float3 _SpecCube0_ProbePosition = gvCameraPosition;
	float3 _SpecCube0_BoxMax = _SpecCube0_ProbePosition+float3(2000, 2000, 2000);
	float3 _SpecCube0_BoxMin = _SpecCube0_ProbePosition-float3(2000, 2000, 2000);
	half3 factors = ((direction > 0 ? _SpecCube0_BoxMax : _SpecCube0_BoxMin) - position) / direction;
	//factors = abs(factors);
	half3 scalar = min(min(factors.x, factors.y), factors.z);
	half3 direc = direction * scalar + (position - _SpecCube0_ProbePosition);
	half3 lightenCol = gtxtSkyCubeTexture.Sample(gssClamp, direc);
	return lightenCol;
}

float4 PSTerrainWater(PixelInputType input) : SV_TARGET
{
	float3 viewDirec = normalize(input.positionW - gvCameraPosition);
	float3 reflecDirec = normalize(reflect(viewDirec, input.normal));
	//reflecDirec = reflect(viewDirec, float3(0,1,0));
	//return float4(reflecDirec, 1);
	float3 reflecCol = BoxProjection_half(reflecDirec, input.positionW);
	//return float4(reflecCol, 1);
	float2 uv = input.uv;
	uv *= 20;
	uv.y += gfCurrentTime * 0.0425f;

	float4 cBaseTexColor = gtxtWaterBaseTexture.SampleLevel(gSamplerState, uv, 0);
	uv.y -= gfCurrentTime * 0.0425f * 1.31f;
	uv.x -= gfCurrentTime * 0.021f;
	float4 cBaseTexColor1 = gtxtWaterBaseTexture.SampleLevel(gSamplerState, uv, 0);
	uv.x += gfCurrentTime * 0.021f * 2;
	float4 cDetail0TexColor = gtxtWaterDetail0Texture.SampleLevel(gSamplerState, uv * 3.0f, 0);
	float4 cDetail1TexColor = gtxtWaterDetail1Texture.SampleLevel(gSamplerState, uv * 5.0f, 0);
	float4 cColor = cBaseTexColor1 * cBaseTexColor * cDetail0TexColor;
	cColor.rgb *= (Lighting(input.position, input.normal).rgb * 3 + float3(0.3, 0.3, 0.3));
	cColor.rgb = saturate(cColor.rgb);
	float alpha = saturate(lerp(-3, 1, 1 - gtxtTerrainAlphaTexture.SampleLevel(gSamplerState, input.uv, 0).a));
	cColor.rgb = lerp(half3(0.1, 0.5, 1), cColor.rgb, alpha);
	half fresnel = pow(1 + dot(input.normal, viewDirec),2.5);
	cColor.rgb = lerp(cColor.rgb * 0.3+ reflecCol*0.7, reflecCol, fresnel);

	half fogDensity = saturate(-0.2+0.8*exp2(-input.uv.z * input.uv.z));
	half3 fogColor = gtxtSkyCubeTexture.SampleLevel(gssClamp, input.positionW - gvCameraPosition, fogDensity * 5+6);
	cColor.rgb = lerp(fogColor, cColor, fogDensity);

	
	cColor.a = lerp(alpha, 1, fresnel);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////
//
struct VS_BILLBOARD_INPUT {
	float3 center : POSITION;
	float2 size : TEXCOORD;
	uint index : TEXTURE;
};

VS_BILLBOARD_INPUT VSBillboard(VS_BILLBOARD_INPUT input) {
	return(input);
}

struct GS_BILLBOARD_GEOMETRY_COLOR_OUTPUT {
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normal : NORMAL;
	float3 uv : TEXCOORD;
	uint index : TEXTURE;
	float3 color : COLOR;
};

static float2 pf2UVs[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

[maxvertexcount(4)]
void GSBillboard(point VS_BILLBOARD_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_COLOR_OUTPUT> outStream) {
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
	float3 f3Right = cross(f3Up, f3Look);
	float fHalfWidth = input[0].size.x * 0.5f;
	float fHalfHeight = input[0].size.y * 0.5f;

	float4 pf4Vertices[4];
	pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);

	GS_BILLBOARD_GEOMETRY_COLOR_OUTPUT output;
	for (int i = 0; i < 4; i++) {
		output.positionW = pf4Vertices[i].xyz;
		output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
		output.normal = f3Look;
		output.uv.xy = pf2UVs[i];
		output.uv.z = output.position.z * 0.0010;
		output.index = input[0].index;
		output.color = VSLighting(output.positionW);

		outStream.Append(output);
	}
}

Texture2D gtxtBillboardTextures[7] : register(t22);

float4 PSBillboard(GS_BILLBOARD_GEOMETRY_COLOR_OUTPUT input) : SV_TARGET
{
	//return float4(1, 0, 1, 1);
	float4 cColor = gtxtBillboardTextures[input.index].Sample(gSamplerState, input.uv);
	if (cColor.a <= 0.3f) discard; //clip(cColor.a - 0.3f);
	cColor.xyz *= input.color;
	half fogDensity = saturate(-0.2+0.8*exp2(-input.uv.z * input.uv.z));
	half3 fogColor = gtxtSkyCubeTexture.SampleLevel(gssClamp, input.positionW - gvCameraPosition, fogDensity * 5+6);
	cColor.rgb = lerp(fogColor, cColor, fogDensity);
	return(cColor);
}
///////////////////////////////////////////////////////////////////////////
/////
struct VS_BILLBOARD_PARTICLE_INPUT {
	float3 center : POSITION;
	float2 size : TEXCOORD;
	float3 velocity : TEXTURE;
};
VS_BILLBOARD_PARTICLE_INPUT VSBillboardParticle(VS_BILLBOARD_PARTICLE_INPUT input)
{
	input.center = mul(float4(input.center,1), gmtxGameObject).xyz;
	input.center += input.velocity * (gfCurrentTime- gfStartTime);
	return (input);
}
struct GS_BILLBOARD_GEOMETRY_OUTPUT {
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	uint index : TEXTURE;
};
//gfCurrentTime
//gf2SheetCount
//gfStartTime :
//gfSheetSpeed
[maxvertexcount(4)]
void GSBillboardParticle(point VS_BILLBOARD_PARTICLE_INPUT input[1], inout TriangleStream<GS_BILLBOARD_GEOMETRY_OUTPUT> outStream) {
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = normalize(gvCameraPosition - input[0].center.xyz);
	float3 f3Right = cross(f3Up, f3Look);
	float fHalfWidth = input[0].size.x * 0.5f;
	float fHalfHeight = input[0].size.y * 0.5f;

	float4 pf4Vertices[4];
	pf4Vertices[0] = float4(input[0].center.xyz + (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[1] = float4(input[0].center.xyz + (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[2] = float4(input[0].center.xyz - (fHalfWidth * f3Right) - (fHalfHeight * f3Up), 1.0f);
	pf4Vertices[3] = float4(input[0].center.xyz - (fHalfWidth * f3Right) + (fHalfHeight * f3Up), 1.0f);

	int idx = (int)((gfCurrentTime - gfStartTime)*gfSheetSpeed);
	idx = idx % (gf2SheetCount.x*gf2SheetCount.y);
	float uvOffsetY = (int)(idx / gf2SheetCount.x) / (gf2SheetCount.y);
	float uvOffsetX = frac(idx / gf2SheetCount.x);

	GS_BILLBOARD_GEOMETRY_OUTPUT output;
	for (int i = 0; i < 4; i++) {
		output.positionW = pf4Vertices[i].xyz;
		output.position = mul(mul(pf4Vertices[i], gmtxView), gmtxProjection);
		output.normal = f3Look;
		output.uv = pf2UVs[i] /gf2SheetCount + float2(uvOffsetX, uvOffsetY);
		output.index = i;

		outStream.Append(output);
	}
}

Texture2D gtxtBillboardParticleTexture : register(t29);

float4 PSBillboardParticle(GS_BILLBOARD_GEOMETRY_OUTPUT input) : SV_TARGET
{
	//return float4(1, 0, 1, 1);
	float4 cColor = gtxtBillboardParticleTexture.Sample(gSamplerState, input.uv);
	if (cColor.a <= 0.3f) discard; //clip(cColor.a - 0.3f);

	return(cColor);
}