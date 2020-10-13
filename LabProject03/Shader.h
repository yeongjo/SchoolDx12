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
	D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob = NULL);

	//�׷��Ƚ� ���������� ���� ��ü�� �����Ѵ�. 
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {} // ��� ���ҽ��� ����� ������
	virtual void ReleaseShaderVariables() {} // ���ҽ� ����
	virtual void ReleaseUploadBuffers() {} // ���ӿ������� ReleaseUploadBuffer ȣ��, �ű⼭ �Ž� ��Ƽ������ ReleaseUploadBuffer ȣ������, ���ҽ����鶧 ����

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) {}
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState = 0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, int nPipelineState = 0);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL) {}
	virtual void ReleaseObjects() {} // ���� ������ ����
	virtual void AnimateObjects(float fTimeElapsed) {}

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

protected:
	ID3DBlob*							m_pd3dVertexShaderBlob = NULL;
	ID3DBlob*							m_pd3dPixelShaderBlob = NULL;

	int									m_nPipelineStates = 0;
	ID3D12PipelineState**				m_ppd3dPipelineStates = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	ID3D12DescriptorHeap*				m_pd3dCbvSrvDescriptorHeap = NULL;

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
	CStandardShader() {}
	virtual ~CStandardShader() {}

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};
////////////////////////////////////////////////////////////////////////////////////
//
class CObjectsShader : public CStandardShader {
public:
	CObjectsShader() { shaderType = ShaderType::ShaderHasObjects; }
	virtual ~CObjectsShader() {}
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext = NULL);
	virtual void ReleaseObjects();

	virtual void					CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);	// Nothing
	virtual void					ReleaseShaderVariables();
	virtual void					ReleaseUploadBuffers();

	virtual void					UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList); // Nothing
	virtual void					AnimateObjects(float fTimeElapsed);
	virtual void					Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
protected:
	CGameObject						**m_ppObjects = NULL;
	int								m_nObjects = 0;
	//ID3D12Resource					*m_pd3dcbGameObjects = NULL;
	//UINT8							*m_pcbMappedGameObjects = NULL;
public:
	//���̴��� ���ԵǾ� �ִ� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�. 
	virtual CGameObject *PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfNearHitDistance);
	virtual CGameObject* GetIntersectObject(const XMFLOAT3& xmf3Position, float *pfNearHitDistance);
};
////////////////////////////////////////////////////////////////////////////////////
class CTerrainShader : public CObjectsShader {
public:
	CTerrainShader() {}
	virtual ~CTerrainShader() {}
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CPlayerShader : public CShader {
public:
	CPlayerShader() {}
	virtual ~CPlayerShader() {}

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};
/////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader {
public:
	CSkyBoxShader() {}
	virtual ~CSkyBoxShader() {}

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();
};

#ifdef INSTANCING_ON
////////////////////////////////////////////////////////////////////////////////////
// �����ϱ� 13
// �Է� ���̾ƿ��� ���� ���� �̿�
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
	//�ν��Ͻ� ���� ���ۿ� ���� ���� ���̴�. 
	ID3D12Resource *m_pd3dcbGameObjects = NULL;
	//VS_VB_INSTANCE *m_pcbMappedGameObjects = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dInstancingBufferView;
};
////////////////////////////////////////////////////////////////////////////////////
// �����ϱ� 14
// ����ȭ�� ���� �̿�
class CInstancingShader2 : public CInstancingShader {
	//m_d3dInstancingBufferView�� �Ⱦ�
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
};
#endif