#pragma once
#include "Shader.h"
#include "Player.h"

#define MAX_LIGHTS				8 
#define MAX_MATERIALS			8 

#define POINT_LIGHT				1
#define SPOT_LIGHT				2
#define DIRECTIONAL_LIGHT		3

struct LIGHT {
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS {
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};

struct TessellationBuffer {
	float tessellationAmount;
	XMFLOAT3 padding;
};

class CScene
{
public:
	CScene() {}
	~CScene(){}

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildDefaultLightsAndMaterials();

	void CreateCbvSrvUavDescriptorHeaps(ID3D12Device* pd3dDevice, int nConstantBufferViews, int nShaderResourceViews,
	                                    int nUnorderedAccessViews);
	void CreateConstantBufferViews(ID3D12Device* pd3dDevice, int nConstantBufferViews, ID3D12Resource* pd3dBuffers,
	                               UINT nStride);
	void CreateUnorderedAccessViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex);
	void CreateUnorderedAccessView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nIndex);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nDescriptorHeapIndex);
	void CreateShaderResourceViews(ID3D12Device* pd3dDevice, int nResources, ID3D12Resource** ppd3dResources,
	                               DXGI_FORMAT* pdxgiSrvFormats);
	void CreateShaderResourceView(ID3D12Device* pd3dDevice, CTexture* pTexture, UINT nIndex);
	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	void ReleaseObjects();
	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void SetComputeRootSignature(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseUploadBuffers();
	ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice, D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags,
	                                         UINT nRootParameters, D3D12_ROOT_PARAMETER* pd3dRootParameters,
	                                         UINT nStaticSamplerDescs,
	                                         D3D12_STATIC_SAMPLER_DESC* pd3dStaticSamplerDescs);

	//씬의 모든 게임 객체들에 대한 마우스 픽킹을 수행한다. 
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);
	CGameObject *GetNearestObject(CGameObject *obj, float * pfNearHitDistance);
	bool RemoveGameObject(CGameObject* obj);

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature* CreateComputeRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature *GetGraphicsRootSignature();
	ID3D12RootSignature* GetComputeRootSignature();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvUavDescriptorHeap->GetCPUDescriptorHandleForHeapStart()); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dCbvSrvUavDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCbvCPUDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCPUDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGPUDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetUavCPUDescriptorStartHandle() { return(m_d3dUavCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetUavGPUDescriptorStartHandle() { return(m_d3dUavGPUDescriptorStartHandle); }

	

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = nullptr;
	ID3D12RootSignature			* m_pd3dComputeRootSignature = nullptr;
	
	CPlayer						*m_pPlayer = nullptr;

	vector<CGameObject*>		m_ppGameObjects;

	vector<CShader*>			m_ppShaders;

	LIGHT						*m_pLights = nullptr;
	int							m_nLights = 0;
	XMFLOAT4					m_xmf4GlobalAmbient;

	ID3D12Resource				*m_pd3dcbLights = nullptr;
	LIGHTS						*m_pcbMappedLights = nullptr;

	CSkyBox						*m_pSkyBox = nullptr;
	CTerrainWater				*pTerrainWater = nullptr;

	WayPointHandler				*_wayPointHandler = nullptr;

	ID3D12Resource				*m_pd3dcbTessellation = nullptr;
	TessellationBuffer			*m_pcbMappedTessellation = nullptr;

	
	ID3D12DescriptorHeap* m_pd3dCbvSrvUavDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorNextHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorNextHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorNextHandle;
};

