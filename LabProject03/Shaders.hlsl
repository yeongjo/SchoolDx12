//���� ��ü�� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbGameObjectInfo : register(b0)
{
matrix gmtxWorld : packoffset(c0);
};
//ī�޶��� ������ ���� ��� ���۸� �����Ѵ�. 
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
	float3 gf3CameraPosition : packoffset(c8);
};
//���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�. 
struct VS_INPUT {
	float3 position : POSITION;
	float4 color : COLOR;
};
//���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
struct VS_OUTPUT {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};
//���� ���̴��� �����Ѵ�. 
VS_OUTPUT VSDiffused(VS_INPUT input) {
	VS_OUTPUT output;
	//������ ��ȯ(���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ)�Ѵ�. 
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView),
		gmtxProjection);
	output.color = input.color;
	return(output);
}
//�ȼ� ���̴��� �����Ѵ�. 
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
	//return float4(1, 0, 0, 1);
	return(input.color);
}

// ====================�ν��Ͻ�========================== 
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

// ====================�ν��Ͻ�2==========================
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

//���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�. 
struct VS_INPUT_LIGHT {
	float3		position : POSITION;
	float3		normal : NORMAL;
	float2		uv : TEXTURECOORD;
};
//���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
struct VS_OUTPUT_LIGHT {
	float4		positionH : SV_POSITION;
	float3		positionW : POSITION;
	float3		normal : NORMAL0;
	float3		normalW : NORMAL1;
	float2		uv : TEXTURECOORD;
};

VS_OUTPUT_LIGHT VSLighting(VS_INPUT_LIGHT input)
{
	VS_OUTPUT_LIGHT output;

	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.positionH = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = mul(float4(input.normal, 0.0f), gmtxWorld).xyz;
	output.normal = input.normal;
	output.uv = input.uv;
	return(output);
}
//�ȼ� ���̴��� �����Ѵ�. 

static float3 gf3AmbientLightColor = float3(0.0f, 0.0f, 0.0f);
static float3 gf3AmbientSpecularColor = float3(0.5f, 0.5f, 0.5f);

static float3 gf3LightDirection = float3(1.4142f, 1.4142f * 0.5f, 1.4142f * 0.5f);
static float3 gf3LightColor = float3(0.98f/2.0f, 0.93f / 2.0f, 0.85f / 2.0f);
static float3 gf3SpecularColor = float3(0.85f, 0.85f, 0.85f);

static float gfSpecular = 2.0f;
static float gfGlossiness = 1.8f;
static float gfSmoothness = 0.95f;
static float gfOneMinusReflectivity = 0.5f;

static float3 gf3ObjectColor = float3(1, 1, 1);

inline float Pow5(float x) {
	return(x * x * x * x * x);
}

inline float3 FresnelTerm(float3 F0, float cosA) {
	return((F0 + (1 - F0) * Pow5(1 - cosA)));
}

inline float3 FresnelLerp(float3 F0, float3 F90, half cosA) {
	return(lerp(F0, F90, Pow5(1 - cosA)));
}

inline float PerceptualRoughnessToSpecPower(float fPerceptualRoughness) {
	float m = fPerceptualRoughness * fPerceptualRoughness;
	float sq = max(1e-4f, m * m);
	float n = (2.0f / sq) - 2.0f;

	return(max(n, 1e-4f));
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float fPerceptualRoughness) {
	float fd90 = 0.5f + 2.0f * LdotH * LdotH * fPerceptualRoughness;
	float fLightScatter = (1.0f + (fd90 - 1.0f) * Pow5(1.0f - NdotL));
	float fViewScatter = (1.0f + (fd90 - 1.0f) * Pow5(1.0f - NdotV));

	return(fLightScatter * fViewScatter);
}

// Smith-Schlick derived for Beckmann
inline float SmithBeckmannVisibilityTerm(float NdotL, float NdotV, float fRoughness) {
	half k = fRoughness * 0.797884560802865f; //0.797884560802865 = sqrt(2.0f / Pi)

	return(1.0f / ((NdotL * (1 - k) + k) * (NdotV * (1 - k) + k) + 1e-5f));
}

inline float NDFBlinnPhongNormalizedTerm(float NdotH, float fRoughnessToSpecPower) {
	float fNormTerm = (fRoughnessToSpecPower + 2.0f) * (0.5f / 3.14159f);
	float fSpecTerm = pow(NdotH, fRoughnessToSpecPower);

	return(fNormTerm * fSpecTerm);
}

float4 PSLighting(VS_OUTPUT_LIGHT input) : SV_TARGET
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 f3Normal = normalize(input.normalW);
	float3 f3ToCamera = normalize(gf3CameraPosition - input.positionW);
	float3 f3Reflection = reflect(f3ToCamera, f3Normal);
	float3 f3Half = normalize(gf3LightDirection + f3ToCamera);

	float nl = saturate(dot(f3Normal, gf3LightDirection));
	float nh = saturate(dot(f3Normal, f3Half));
	float nv = saturate(dot(f3Normal, f3ToCamera));
	float lv = saturate(dot(gf3LightDirection, f3ToCamera));
	float lh = saturate(dot(gf3LightDirection, f3Half));

	float fPerceptualRoughness = 1.0f - gfSmoothness;
	float fDiffuseTerm = DisneyDiffuse(nv, nl, lh, fPerceptualRoughness) * nl;

	float fRoughness = fPerceptualRoughness * fPerceptualRoughness;
	float V = SmithBeckmannVisibilityTerm(nl, nv, fRoughness);
	float D = NDFBlinnPhongNormalizedTerm(nh, PerceptualRoughnessToSpecPower(fPerceptualRoughness));
	float fSpecularTerm = max(0.0f, V * D * 3.14159f * nl);

	float fSurfaceReduction = 1.0f / ((fRoughness * fRoughness) + 1.0f);

	float fGrazingTerm = saturate(gfSmoothness + (1 - gfOneMinusReflectivity));
	cColor.rgb = gf3ObjectColor * (gf3AmbientLightColor + (gf3LightColor * fDiffuseTerm));
	cColor.rgb += fSpecularTerm * gf3LightColor * FresnelTerm(gf3SpecularColor, lh);
	cColor.rgb += fSurfaceReduction * gf3AmbientSpecularColor * FresnelLerp(gf3SpecularColor, fGrazingTerm, nv);

	//cColor.rgb += saturate(float3(0.25f * abs(input.uv.r), 0.25f * abs(input.uv.g), 0.25f)) * 0.5f;

	float fRim = saturate(dot(f3ToCamera, f3Normal));
	cColor.rgb += float3(0.25f, 0.0f, 0.0f) * pow(fRim, 4.0f);

	return(cColor);
}