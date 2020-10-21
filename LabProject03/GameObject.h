﻿#pragma once
#include "Mesh.h"
#include "Camera.h"

#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

class CTexturedShader;
class ObjectSpawner;
class CObjectsShader;
class CShader;
class CStandardShader;
class CGameObject;

class CTexture : public CRef {
public:
	CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
	virtual ~CTexture();

private:
	UINT							m_nTextureType;

	int								m_nTextures = 0;
	_TCHAR							(*m_ppstrTextureNames)[64] = nullptr;
	ID3D12Resource**				m_ppd3dTextures = nullptr;
	ID3D12Resource**				m_ppd3dTextureUploadBuffers;

	UINT*							m_pnResourceTypes = nullptr;

	DXGI_FORMAT*					m_pdxgiBufferFormats = nullptr;
	int*							m_pnBufferElements = nullptr;

	int								m_nRootParameters = 0;
	int*							m_pnRootParameterIndices = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSrvGpuDescriptorHandles = nullptr;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE*	m_pd3dSamplerGpuDescriptorHandles = nullptr;

public:
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
	void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
	ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

	int LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader, UINT nIndex);

	void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
	void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

	int GetRootParameters() { return(m_nRootParameters); }
	int GetTextures() { return(m_nTextures); }
	_TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
	ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
	int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

	UINT GetTextureType() { return(m_nTextureType); }
	UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
	DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
	int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

	void ReleaseUploadBuffers();
};

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40


class CMaterial : public CRef {
public:
	CMaterial();
	virtual ~CMaterial();

	void SetShader(CShader *pShader);
	void SetMaterialType(UINT nType) { m_nType |= nType; }
	void SetTexture(CTexture* pTexture);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	CShader		*m_pShader = nullptr;
	CTexture*	m_pTexture = nullptr;

	XMFLOAT4	m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4	m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4	m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4	m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	UINT		m_nType = 0x00;

	float		m_fGlossiness = 0.0f;
	float		m_fSmoothness = 0.0f;
	float		m_fSpecularHighlight = 0.0f;
	float		m_fMetallic = 0.0f;
	float		m_fGlossyReflection = 0.0f;
};


class CGameObject : public CRef {
public:
	CGameObject();
	CGameObject(int nMaterials);
	virtual ~CGameObject();

	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = nullptr);
	virtual void PrepareAnimate() {}
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = nullptr);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView);

	virtual bool IsVisible(CCamera *pCamera = nullptr);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, CMaterial *pMaterial){}

	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View,
		XMFLOAT3 *pxmf3PickRayOrigin, XMFLOAT3 *pxmf3PickRayDirection);
	virtual int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfHitDistance);
	virtual float GetDistance(const XMFLOAT3& position);

	
	CGameObject *GetParent() { return(m_pParent); }
	CGameObject *FindFrame(char *pstrFrameName);
	int FindReplicatedTexture(_TCHAR* pstrTextureName, D3D12_GPU_DESCRIPTOR_HANDLE* pd3dSrvGpuDescriptorHandle);
	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	CGameObject* AddSibling(CGameObject* obj = nullptr);

	void SetMesh(CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetShader(int nMaterial, CShader *pShader);
	void SetMaterial(int nMaterial, CMaterial *pMaterial);
	void SetChild(CGameObject *pChild);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Scale(float x, float y, float z);

	void LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader);

	static CGameObject *LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader);
	static CGameObject *LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader);

	static void PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent);

	char				m_pstrFrameName[64];
	CMesh				*m_pMesh = nullptr;
	int					m_nMaterials = 0;
	CMaterial			**m_ppMaterials = nullptr;

	XMFLOAT4X4			m_xmf4x4Transform;
	XMFLOAT4X4			m_xmf4x4World;

	CGameObject 		*m_pParent = nullptr;
	CGameObject 		*m_pChild = nullptr;
	CGameObject 		*m_pSibling = nullptr;

	bool				isDestroyNextFrame = false;
};

//========================================================================
//========================================================================
//========================================================================

class CSuperCobraObject : public CGameObject {
public:
	CSuperCobraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSuperCobraObject();

private:
	CGameObject					*m_pMainRotorFrame = nullptr;
	CGameObject					*m_pTailRotorFrame = nullptr;

public:
	void PrepareAnimate() override;
	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr) override;
};

class CGunshipObject : public CGameObject {
public:
	CGunshipObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CGunshipObject();

private:
	CGameObject					*m_pMainRotorFrame = nullptr;
	CGameObject					*m_pTailRotorFrame = nullptr;

public:
	void PrepareAnimate() override;
	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr) override;
};

class CMi24Object : public CGameObject {
public:
	CMi24Object(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CMi24Object();

private:
	CGameObject					*m_pMainRotorFrame = nullptr;
	CGameObject					*m_pTailRotorFrame = nullptr;

public:
	void PrepareAnimate() override;
	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr) override;
};

//========================================================================
//========================================================================

class CSkyBox : public CGameObject {
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = nullptr) override;
};

//========================================================================
//========================================================================

class CRotatingObject : public CGameObject {
public:
	CRotatingObject();
	virtual ~CRotatingObject() = default;
protected:
	XMFLOAT3 m_xmf3RotationAxis;
public:
	float m_fRotationSpeed;

	void SetRotationSpeed(float fRotationSpeed) {
		m_fRotationSpeed = fRotationSpeed;
	}
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) {
		m_xmf3RotationAxis = xmf3RotationAxis;
	}
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);
};

class CFollowObject : public CRotatingObject {
public:
	float elapsedTime = 0;
	CGameObject* followObj = nullptr;
	float speed = 5;
	void(*OnArrived)(CFollowObject* self, CGameObject* target) = nullptr;
	
	CFollowObject() :CRotatingObject() { }
	virtual ~CFollowObject() { }

	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL) override;
};
/////////////////////////////////////////////////////////////////////////////////////
//
#define HIGHTMAPTERRAINOBJ_ON
#ifdef HIGHTMAPTERRAINOBJ_ON
class CHeightMapTerrain : public CGameObject {
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage					*m_pHeightMapImage;

	int								m_nWidth;
	int								m_nLength;

	XMFLOAT3						m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainWater : public CGameObject {
public:
	CTerrainWater(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float nWidth, float nLength);
	//	CTerrainWater(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale);
	virtual ~CTerrainWater();

private:
	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	XMFLOAT4X4					m_xmf4x4Texture;

	//	virtual void Animate(float fTimeElapsed);
	//	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};
#endif
class CExplosibleObject : public CRotatingObject {
public:
	bool isExploed = false;
	float elapsedTime = 0;
	vector<CFollowObject> children;
	bool firstExplosion = true;
	XMFLOAT3 direction;
	CExplosibleObject(CMesh* mesh) {
		m_pMesh = mesh;
		direction = XMFLOAT3(random()*0.2f, random()*0.2f, random()*0.2f);
	}
	virtual ~CExplosibleObject() {
	}

	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) override;

	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) override {
		if (isExploed) {
			for (size_t i = 0; i < children.size(); i++) {
				children[i].Render(pd3dCommandList, pCamera);
			}
			return;
		}
		CRotatingObject::Render(pd3dCommandList, pCamera);
	}
};
class CMapObject : public CGameObject, public Singleton<CMapObject> {
public:
	CMapObject() :CGameObject() {
	}
	virtual ~CMapObject() {
	}
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = NULL);

	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfHitDistance) override {
		return 0;
	}

	float GetDistance(const XMFLOAT3& position) override {
		return FLT_MAX;
	}

	bool IsVisible(CCamera *pCamera = nullptr) override {
		return true;
	}
};

class WayPointFollower : public CGameObject {
public:
	float prevPos = 0;
	float pos = 0;
	float direc = 1;
	float canMove = 1;
	float speed = Random(0.006f, 0.008f);
	float attackRange = Random(0.04f, 0.06f);
	float attackDelay = 1;
	float durability = 5; // 내구도
	ObjectSpawner* objectSpawner;

	XMFLOAT3 offset;

	void Destroy();
	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr) override;
};

class ObjectSpawner : public CGameObject {
public:
	vector<CGameObject*>	baseObjs;
	CObjectsShader			*targetShader;
	CTexturedShader			*bulletShader;
	float					remainSpawnTime0=0;
	float					remainSpawnTime1=0;
	CFollowObject			*bulletObj;

	vector<WayPointFollower*>	Objs0;
	vector<WayPointFollower*>	Objs1;

	void InitBullet(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);

	void AddBullet(WayPointFollower* owner, WayPointFollower* target);
	void AddObject(int idx, XMFLOAT3 off = XMFLOAT3(0,0,0), bool isReverse = false);
	void Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent = nullptr) override;
};