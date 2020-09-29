#pragma once
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20
#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject {
protected:
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다. 
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;
	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다. 
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;
	//플레이어의 이동 속도를 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Velocity;
	//플레이어에 작용하는 중력을 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Gravity;
	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다. 
	float m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다. 
	float m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다. 
	float m_fFriction;
	//플레이어의 위치가 바뀔 때마다 호출되는 OnPlayerUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pPlayerUpdatedContext;
	//카메라의 위치가 바뀔 때마다 호출되는 OnCameraUpdateCallback() 함수에서 사용하는 데이터이다. 
	LPVOID m_pCameraUpdatedContext;
	//플레이어에 현재 설정된 카메라이다. 
	CCamera *m_pCamera = NULL;
public:
	CPlayer(int nMeshes = 1);
	virtual ~CPlayer();
	XMFLOAT3 GetPosition() {
		return(m_xmf3Position);
	}
	XMFLOAT3 GetLookVector() {
		return(m_xmf3Look);
	}
	XMFLOAT3 GetUpVector() {
		return(m_xmf3Up);
	}
	XMFLOAT3 GetRightVector() {
		return(m_xmf3Right);
	}
	void SetFriction(float fFriction) {
		m_fFriction = fFriction;
	}
	void SetGravity(const XMFLOAT3& xmf3Gravity) {
		m_xmf3Gravity = xmf3Gravity;
	}
	void SetMaxVelocityXZ(float fMaxVelocity) {
		m_fMaxVelocityXZ = fMaxVelocity;
	}
	void SetMaxVelocityY(float fMaxVelocity) {
		m_fMaxVelocityY = fMaxVelocity;
	}
	void SetVelocity(const XMFLOAT3& xmf3Velocity) {
		m_xmf3Velocity = xmf3Velocity;
	}
	/*플레이어의 위치를 xmf3Position 위치로 설정한다. xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다. 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/
	void SetPosition(const XMFLOAT3& xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x -
			m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z),
			false);
	}
	XMFLOAT3& GetVelocity() {
		return(m_xmf3Velocity);
	}
	float GetYaw() {
		return(m_fYaw);
	}
	float GetPitch() {
		return(m_fPitch);
	}
	float GetRoll() {
		return(m_fRoll);
	}
	CCamera *GetCamera() {
		return(m_pCamera);
	}
	void SetCamera(CCamera *pCamera) {
		m_pCamera = pCamera;
	}
	//플레이어를 이동하는 함수이다. 
	void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//플레이어를 회전하는 함수이다. 
	void Rotate(float x, float y, float z);
	//플레이어의 위치와 회전 정보를 경과 시간에 따라 갱신하는 함수이다. 
	virtual void Update(float fTimeElapsed);
	//플레이어의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {
	}
	void SetPlayerUpdatedContext(LPVOID pContext) {
		m_pPlayerUpdatedContext = pContext;
	}
	//카메라의 위치가 바뀔 때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) {
		m_pCameraUpdatedContext = pContext;
	}
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	//카메라를 변경하기 위하여 호출하는 함수이다. 
	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(NULL);
	}
	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다. 
	virtual void OnPrepareRender();
	//플레이어의 카메라가 3인칭 카메라일 때 플레이어(메쉬)를 렌더링한다. 
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera =
	NULL);
};
class CAirplanePlayer : public CPlayer {
protected:
	CMesh* bulletMesh;
public:
	vector<CGameObject*> child;
public:
	CAirplanePlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, int nMeshes = 1);
	virtual ~CAirplanePlayer();
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void OnPlayerUpdateCallback(float fTimeElapsed) {
		XMFLOAT3 xmf3PlayerPosition = GetPosition();
		CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
		/*지형에서 플레이어의 현재 위치 (x, z)의 지형 높이(y)를 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의
		중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/
		float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) +
			6.0f;
		/*플레이어의 위치 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점
		작아지게 된다. 이때 플레이어의 현재 위치 벡터의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어
		의 일부가 지형 아래에 있게 된다. 이러한 경우를 방지하려면 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어
		의 위치 벡터의 y-값을 지형의 높이(실제로 지형의 높이 + 6)로 설정한다. 그러면 플레이어는 항상 지형 위에 있게 된
		다.*/
		if (xmf3PlayerPosition.y < fHeight) {
			XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
			xmf3PlayerVelocity.y = 0.0f;
			SetVelocity(xmf3PlayerVelocity);
			xmf3PlayerPosition.y = fHeight;
			SetPosition(xmf3PlayerPosition);
		}
	}

	void OnCameraUpdateCallback(float fTimeElapsed) {
		XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
		/*높이 맵에서 카메라의 현재 위치 (x, z)에 대한 지형의 높이(y 값)를 구한다. 이 값이 카메라의 위치 벡터의 y-값 보
		다 크면 카메라가 지형의 아래에 있게 된다. 이렇게 되면 다음 그림의 왼쪽과 같이 지형이 그려지지 않는 경우가 발생
		한다(카메라가 지형 안에 있으므로 삼각형의 와인딩 순서가 바뀐다). 이러한 경우가 발생하지 않도록 카메라의 위치 벡
		터의 y-값의 최소값은 (지형의 높이 + 5)로 설정한다. 카메라의 위치 벡터의 y-값의 최소값은 지형의 모든 위치에서
		카메라가 지형 아래에 위치하지 않도록 설정해야 한다.*/
		CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
		float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) +
			5.0f;
		if (xmf3CameraPosition.y <= fHeight) {
			xmf3CameraPosition.y = fHeight;
			m_pCamera->SetPosition(xmf3CameraPosition);
			if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA) {
				//3인칭 카메라의 경우 카메라 위치(y-좌표)가 변경되었으므로 카메라가 플레이어를 바라보도록 한다.
				CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera;
				p3rdPersonCamera->SetLookAt(GetPosition());
			}
		}
	}

	void Shot(CGameObject* obj);
	virtual void Update(float fTimeElapsed);
};

class CTerrainPlayer : public CPlayer {
public:
	CTerrainPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes = 1);
	virtual ~CTerrainPlayer();
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	virtual void OnCameraUpdateCallback(float fTimeElapsed);
};