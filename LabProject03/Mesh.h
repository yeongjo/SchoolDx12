#pragma once
class CRef {
	int	m_nReferences = 0;
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
};

#define VERTEXT_POSITION				0x01
#define VERTEXT_COLOR					0x02
#define VERTEXT_NORMAL					0x04
#define VERTEXT_TANGENT					0x08
#define VERTEXT_TEXTURE_COORD0			0x10
#define VERTEXT_TEXTURE_COORD1			0x20

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)

class CVertex {
public:
	XMFLOAT3						m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {}
};

class CDiffusedVertex : public CVertex {
public:
	XMFLOAT4						m_xmf4Diffuse;

public:
	CDiffusedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); }
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse; }
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)) { m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse; }
	~CDiffusedVertex() {}
};

class CDiffused2TexturedVertex : public CDiffusedVertex {
public:
	XMFLOAT2						m_xmf2TexCoord0;
	XMFLOAT2						m_xmf2TexCoord1;
	XMFLOAT3						m_xmf3Normal;

public:
	CDiffused2TexturedVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); m_xmf2TexCoord0 = m_xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f); }
	CDiffused2TexturedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse, XMFLOAT2 xmf2TexCoord0, XMFLOAT2 xmf2TexCoord1) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1; }
	CDiffused2TexturedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT2 xmf2TexCoord0 = XMFLOAT2(0.0f, 0.0f), XMFLOAT2 xmf2TexCoord1 = XMFLOAT2(0.0f, 0.0f)) { m_xmf3Position = xmf3Position; m_xmf4Diffuse = xmf4Diffuse; m_xmf2TexCoord0 = xmf2TexCoord0; m_xmf2TexCoord1 = xmf2TexCoord1; }
	~CDiffused2TexturedVertex() {}
};
class CGeometryBillboardParticleVertex : public CVertex {
public:
	XMFLOAT2						m_xmf2Size;
	XMFLOAT3						m_xmf2Velocity;
	CGeometryBillboardParticleVertex() = default;
	~CGeometryBillboardParticleVertex() = default;
};
class CGeometryBillboardVertex : public CVertex {
public:
	XMFLOAT2						m_xmf2Size;
	UINT							m_nTexture;

public:
	CGeometryBillboardVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf2Size = XMFLOAT2(5.0f, 10.0f); m_nTexture = 0; }
	CGeometryBillboardVertex(float x, float y, float z, XMFLOAT2 xmf2Size = XMFLOAT2(5.0f, 10.0f), UINT nTexture = 0) { m_xmf3Position = XMFLOAT3(x, y, z); m_xmf2Size = xmf2Size; m_nTexture = nTexture; }
	CGeometryBillboardVertex(XMFLOAT3 xmf3Position, XMFLOAT2 xmf2Size = XMFLOAT2(5.0f, 10.0f), UINT nTexture = 0) { m_xmf3Position = xmf3Position; m_xmf2Size = xmf2Size; m_nTexture = nTexture; }
	~CGeometryBillboardVertex() {}
};

class CMesh : public CRef {
public:
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMesh();

	UINT GetType() { return(m_nType); }

	virtual int GetIntersectRayCount(XMFLOAT3& xmf3RayOrigin, XMFLOAT3& xmf3RayDirection, float *pfNearHitDistance);
	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet);
protected:
	char							m_pstrMeshName[256] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	BoundingBox						m_xmBoundingBox;
	
	UINT							m_nSlot = 0;
	UINT							m_nOffset = 0;
	
	XMFLOAT3						*m_pxmf3Positions = nullptr;

	ID3D12Resource					*m_pd3dPositionBuffer = nullptr;
	ID3D12Resource					*m_pd3dPositionUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	int								m_nSubMeshes = 0;
	int								*m_pnSubSetIndices = nullptr;
	UINT							**m_ppnSubSetIndices = nullptr;

	ID3D12Resource					**m_ppd3dSubSetIndexBuffers = nullptr;
	ID3D12Resource					**m_ppd3dSubSetIndexUploadBuffers = nullptr;
	D3D12_INDEX_BUFFER_VIEW*		m_pd3dSubSetIndexBufferViews = nullptr;
public:
	int								m_nVertices = 0;
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxMesh : public CMesh {
public:
	CSkyBoxMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CTexturedRectMesh : public CMesh {
public:
	CTexturedRectMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth = 20.0f, float fHeight = 0.0f, float fDepth = 20.0f, float fxPosition = 0.0f, float fyPosition = 0.0f, float fzPosition = 0.0f, float divX = 1, float divY = 1, float divZ = 1);
	virtual ~CTexturedRectMesh();

protected:
	XMFLOAT2						*m_pxmf2TextureCoords0 = nullptr;

	ID3D12Resource					*m_pd3dTextureCoord0Buffer = nullptr;
	ID3D12Resource					*m_pd3dTextureCoord0UploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

public:
	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet) override;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CStandardMesh : public CMesh {
public:
	CStandardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CStandardMesh();

protected:
	XMFLOAT4						*m_pxmf4Colors = nullptr;
	XMFLOAT3						*m_pxmf3Normals = nullptr;
	XMFLOAT3						*m_pxmf3Tangents = nullptr;
	XMFLOAT3						*m_pxmf3BiTangents = nullptr;
	XMFLOAT2						*m_pxmf2TextureCoords0 = nullptr;
	XMFLOAT2						*m_pxmf2TextureCoords1 = nullptr;

	ID3D12Resource					*m_pd3dTextureCoord0Buffer = nullptr;
	ID3D12Resource					*m_pd3dTextureCoord0UploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	ID3D12Resource					*m_pd3dTextureCoord1Buffer = nullptr;
	ID3D12Resource					*m_pd3dTextureCoord1UploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

	ID3D12Resource					*m_pd3dNormalBuffer = nullptr;
	ID3D12Resource					*m_pd3dNormalUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	ID3D12Resource					*m_pd3dTangentBuffer = nullptr;
	ID3D12Resource					*m_pd3dTangentUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	ID3D12Resource					*m_pd3dBiTangentBuffer = nullptr;
	ID3D12Resource					*m_pd3dBiTangentUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBiTangentBufferView;

public:
	void LoadMeshFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, FILE *pInFile);

	void ReleaseUploadBuffers() override;
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, int nSubSet) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapImage {
private:
	BYTE						*m_pHeightMapPixels;

	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	XMFLOAT3 GetHeightMapNormal(int x, int z);
	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE GetRawImagePixel(int x, int z) { return(m_pHeightMapPixels[x + (z*m_nWidth)]); }
	BYTE *GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh {
protected:
	int							m_nWidth;
	int							m_nLength;
	XMFLOAT3					m_xmf3Scale;

public:
	CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void *pContext = nullptr);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};

class CGeometryBillboardMesh : public CMesh {
public:
	CGeometryBillboardMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pVertices, UINT nVertices, UINT nStride);
	virtual ~CGeometryBillboardMesh();
};