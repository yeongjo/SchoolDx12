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
	//������ ���콺�� Ű���� �޽����� ó���Ѵ�. 
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

	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�. 
	ID3D12RootSignature			*CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	ID3D12RootSignature			*GetGraphicsRootSignature();

	CPlayer						*m_pPlayer = NULL;
protected:
	CHeightMapTerrain			*m_pTerrain = NULL;
public:
	CHeightMapTerrain			*GetTerrain() { return(m_pTerrain); }

protected:
	//���� ���� ��ü���� �����̴�. ���� ��ü�� ���̴��� �����Ѵ�. 
	CGameObject					**m_ppObjects = NULL;
	int							m_nObjects = 0;

	//��ġ(Batch) ó���� �ϱ� ���Ͽ� ���� ���̴����� ����Ʈ�� ǥ���Ѵ�. 
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
	//���� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�. 
	CGameObject					*PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);
	CGameObject					*GetIntersectObject(const XMFLOAT3 & xmf3Position, float * pfNearHitDistance);
	bool						RemoveGameObject(CGameObject* obj);
};

