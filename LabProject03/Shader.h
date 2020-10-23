#pragma once
#include "Camera.h"
#include "GameObject.h"

enum ShaderType {
	ShaderHasObjects,
	ObjectHasShader
};

class CShader : public CRef {
public:
	CShader();
	virtual ~CShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
	virtual D3D12_SHADER_BYTECODE CreateHullShader();
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();
	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob = nullptr);

	//그래픽스 파이프라인 상태 객체를 생성한다. 
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {} // 상수 리소스를 만들고 맵핑함
	virtual void ReleaseShaderVariables() {} // 리소스 해제
	virtual void ReleaseUploadBuffers() {} // 게임오브젝의 ReleaseUploadBuffer 호출, 거기서 매쉬 머티리얼의 ReleaseUploadBuffer 호출해줌, 리소스만들때 생김

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) {}
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0);
	virtual void AnimateObjects(float fTimeElapsed) {}

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = nullptr) {}
	virtual void ReleaseObjects() {} // 만든 오브젝 지움

	void CreateCbvSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex, UINT nRootParameterStartIndex);
	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, int nIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }

	ShaderType GetType() { return shaderType; }
private:
	D3D12_SHADER_BYTECODE CreateEmptyShader();
	
public:
	D3D12_PRIMITIVE_TOPOLOGY_TYPE		m_d3dTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
protected:
	wstring								name = L"shader";
	
	ID3DBlob*							m_pd3dVertexShaderBlob = nullptr;
	ID3DBlob*							m_pd3dPixelShaderBlob = nullptr;
	ID3DBlob*							m_pd3dHullShaderBlob = nullptr;
	ID3DBlob*							m_pd3dDomainShaderBlob = nullptr;
	ID3DBlob*							m_pd3dGeometryShaderBlob = nullptr;

	int									m_nPipelineStates = 0;
	ID3D12PipelineState**				m_ppd3dPipelineStates = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	ID3D12DescriptorHeap*				m_pd3dCbvSrvDescriptorHeap = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE			m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE			m_d3dSrvGPUDescriptorNextHandle;

	ShaderType							shaderType = ShaderType::ObjectHasShader;

};
/////////////////////////////////////////////////////////////////////////////////////
//
class CStandardShader : public CShader {
public:
	CStandardShader() = default;
	virtual ~CStandardShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
};
////////////////////////////////////////////////////////////////////////////////////
//
class CObjectsShader : public CStandardShader {
public:
	CObjectsShader() { shaderType = ShaderType::ShaderHasObjects;}
	virtual ~CObjectsShader() = default;
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = nullptr) override;
	void ReleaseObjects() override;

	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) override;	// Nothing
	void ReleaseShaderVariables() override;
	void ReleaseUploadBuffers() override;
	
	void AnimateObjects(float fTimeElapsed) override;
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0) override;

	//셰이더에 포함되어 있는 모든 게임 객체들에 대한 마우스 픽킹을 수행한다. 
	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfNearHitDistance);
	virtual CGameObject* GetIntersectObject(CGameObject *obj, float *pfNearHitDistance);
	
	vector<CGameObject*>			m_ppObjects;
protected:
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList); // Nothing
	
};
////////////////////////////////////////////////////////////////////////////////////
class CTexturedShader : public CObjectsShader {
public:
	CTexturedShader() = default;
	virtual ~CTexturedShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};
////////////////////////////////////////////////////////////////////////////////////
class CBillboardObjectsShader : public CShader {
public:
	CBillboardObjectsShader() { name = L"CBillboardObjectsShader"; }
	virtual ~CBillboardObjectsShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	D3D12_RASTERIZER_DESC CreateRasterizerState() override;

	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
	D3D12_SHADER_BYTECODE CreateGeometryShader() override;

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = nullptr);
	void ReleaseObjects() override;
	void ReleaseUploadBuffers() override;

	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0) override;

	CGeometryBillboardMesh*			m_pGeometryBillboardMesh = nullptr;
	CTexture*						m_pBillboardTexture = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////
///

struct ParticleInfo {
	XMFLOAT3 pos;
	float startTime = -1000;
	float sheetSpeed = 3;
	float lifeTime = 1;
	int maxParticle = 10;
};

///
class CBillboardParticleShader : public CBillboardObjectsShader {
public:
	CBillboardParticleShader() { name = L"CBillboardParticleShader"; }
	virtual ~CBillboardParticleShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
	D3D12_SHADER_BYTECODE CreateGeometryShader() override;

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = nullptr) override;

	static void AddParticle(XMFLOAT3 xmf3Pos, float lifeTime = 1, int maxParticle = 10);

	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0) override;

	static const UINT	MAX_PARTICLE_COUNT = 32;
	static ParticleInfo		m_pParticle_info[MAX_PARTICLE_COUNT];
	//CGeometryBillboardMesh*			m_pGeometryBillboardMesh = NULL;
	//CTexture*						m_pBillboardTexture = NULL;
};
////////////////////////////////////////////////////////////////////////////////////
class CTerrainShader : public CTexturedShader {
public:
	CTerrainShader() { name = L"CTerrainShader"; }
	virtual ~CTerrainShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;

	void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) override;
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CShader {
public:
	CPlayerShader() = default;
	virtual ~CPlayerShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader {
public:
	CSkyBoxShader() = default;
	virtual ~CSkyBoxShader() = default;

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
};

#ifdef INSTANCING_ON
////////////////////////////////////////////////////////////////////////////////////
// 따라하기 13
// 입력 레이아웃과 정정 버퍼 이용
class CInstancingShader : public CObjectsShader {
public:
	CInstancingShader();
	virtual ~CInstancingShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature
		*pd3dGraphicsRootSignature);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList, void *pContext);
	virtual void ReleaseObjects() {
	}
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
protected:
	//인스턴스 정점 버퍼와 정점 버퍼 뷰이다. 
	ID3D12Resource *m_pd3dcbGameObjects = NULL;
	//VS_VB_INSTANCE *m_pcbMappedGameObjects = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
};
////////////////////////////////////////////////////////////////////////////////////
// 따라하기 14
// 구조화된 버퍼 이용
class CInstancingShader2 : public CInstancingShader {
	//m_d3dInstancingBufferView는 안씀
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
};
#endif

////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainWaterShader : public CShader {
public:
	CTerrainWaterShader() = default;
	virtual ~CTerrainWaterShader() = default;

	D3D12_BLEND_DESC CreateBlendState() override;
	D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	D3D12_SHADER_BYTECODE CreateVertexShader() override;
	D3D12_SHADER_BYTECODE CreatePixelShader() override;
	D3D12_SHADER_BYTECODE CreateGeometryShader() override;
	virtual D3D12_SHADER_BYTECODE CreateHullShader();
	virtual D3D12_SHADER_BYTECODE CreateDomainShader();
};