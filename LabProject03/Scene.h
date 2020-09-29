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
	XMFLOAT4				test;
};

struct LIGHTS {
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
};

struct MATERIALS {
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

class CScene
{
public:
	CScene();
	~CScene();
	//씬에서 마우스와 키보드 메시지를 처리한다. 
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildLightsAndMaterials();

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext);
	void ReleaseObjects();
	bool ProcessInput(UCHAR *pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void ReleaseUploadBuffers();

	//그래픽 루트 시그너쳐를 생성한다. 
	ID3D12RootSignature			*CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature			*GetGraphicsRootSignature();

	CPlayer						*m_pPlayer = NULL;
protected:
	CHeightMapTerrain			*m_pTerrain = NULL;
public:
	CHeightMapTerrain			*GetTerrain() { return(m_pTerrain); }

protected:
	//씬은 게임 객체들의 집합이다. 게임 객체는 셰이더를 포함한다. 
	CGameObject					**m_ppObjects = NULL;
	int							m_nObjects = 0;

	//배치(Batch) 처리를 하기 위하여 씬을 셰이더들의 리스트로 표현한다. 
	CObjectsShader				**m_pShaders = NULL;
	int							m_nShaders = 0;

	LIGHTS						*m_pLights = NULL;

	ID3D12Resource				*m_pd3dcbLights = NULL;
	LIGHTS						*m_pcbMappedLights = NULL;

	MATERIALS					*m_pMaterials = NULL;

	ID3D12Resource				*m_pd3dcbMaterials = NULL;
	MATERIAL					*m_pcbMappedMaterials = NULL;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;
public:
	//씬의 모든 게임 객체들에 대한 마우스 픽킹을 수행한다. 
	CGameObject					*PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);
	CGameObject					*GetIntersectObject(const XMFLOAT3 & xmf3Position, float * pfNearHitDistance);
	bool						RemoveGameObject(CGameObject* obj);
};

