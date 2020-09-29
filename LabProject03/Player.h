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
	//�÷��̾��� ��ġ ����, x-��(Right), y-��(Up), z-��(Look) �����̴�. 
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;
	//�÷��̾ ���� x-��(Right), y-��(Up), z-��(Look)���� �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����. 
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;
	//�÷��̾��� �̵� �ӵ��� ��Ÿ���� �����̴�. 
	XMFLOAT3 m_xmf3Velocity;
	//�÷��̾ �ۿ��ϴ� �߷��� ��Ÿ���� �����̴�. 
	XMFLOAT3 m_xmf3Gravity;
	//xz-��鿡�� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����. 
	float m_fMaxVelocityXZ;
	//y-�� �������� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����. 
	float m_fMaxVelocityY;
	//�÷��̾ �ۿ��ϴ� �������� ��Ÿ����. 
	float m_fFriction;
	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdateCallback() �Լ����� ����ϴ� �������̴�. 
	LPVOID m_pPlayerUpdatedContext;
	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnCameraUpdateCallback() �Լ����� ����ϴ� �������̴�. 
	LPVOID m_pCameraUpdatedContext;
	//�÷��̾ ���� ������ ī�޶��̴�. 
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
	/*�÷��̾��� ��ġ�� xmf3Position ��ġ�� �����Ѵ�. xmf3Position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� xmf3Position ���������� ���Ͱ� �ȴ�. ���� �÷��̾��� ��ġ���� �� ���� ��ŭ �̵��Ѵ�.*/
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
	//�÷��̾ �̵��ϴ� �Լ��̴�. 
	void Move(DWORD nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	//�÷��̾ ȸ���ϴ� �Լ��̴�. 
	void Rotate(float x, float y, float z);
	//�÷��̾��� ��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�. 
	virtual void Update(float fTimeElapsed);
	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {
	}
	void SetPlayerUpdatedContext(LPVOID pContext) {
		m_pPlayerUpdatedContext = pContext;
	}
	//ī�޶��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�. 
	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) {
		m_pCameraUpdatedContext = pContext;
	}
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	//ī�޶� �����ϱ� ���Ͽ� ȣ���ϴ� �Լ��̴�. 
	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return(NULL);
	}
	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�. 
	virtual void OnPrepareRender();
	//�÷��̾��� ī�޶� 3��Ī ī�޶��� �� �÷��̾�(�޽�)�� �������Ѵ�. 
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
		/*�������� �÷��̾��� ���� ��ġ (x, z)�� ���� ����(y)�� ���Ѵ�. �׸��� �÷��̾� �޽��� ���̰� 12�̰� �÷��̾���
		�߽��� ������ü�� ����̹Ƿ� y ���� �޽��� ������ ������ ���ϸ� �÷��̾��� ��ġ�� �ȴ�.*/
		float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) +
			6.0f;
		/*�÷��̾��� ��ġ ������ y-���� �����̸�(���� ���, �߷��� ����Ǵ� ���) �÷��̾��� ��ġ ������ y-���� ����
		�۾����� �ȴ�. �̶� �÷��̾��� ���� ��ġ ������ y ���� ������ ����(������ ������ ���� + 6)���� ������ �÷��̾�
		�� �Ϻΰ� ���� �Ʒ��� �ְ� �ȴ�. �̷��� ��츦 �����Ϸ��� �÷��̾��� �ӵ� ������ y ���� 0���� ����� �÷��̾�
		�� ��ġ ������ y-���� ������ ����(������ ������ ���� + 6)�� �����Ѵ�. �׷��� �÷��̾�� �׻� ���� ���� �ְ� ��
		��.*/
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
		/*���� �ʿ��� ī�޶��� ���� ��ġ (x, z)�� ���� ������ ����(y ��)�� ���Ѵ�. �� ���� ī�޶��� ��ġ ������ y-�� ��
		�� ũ�� ī�޶� ������ �Ʒ��� �ְ� �ȴ�. �̷��� �Ǹ� ���� �׸��� ���ʰ� ���� ������ �׷����� �ʴ� ��찡 �߻�
		�Ѵ�(ī�޶� ���� �ȿ� �����Ƿ� �ﰢ���� ���ε� ������ �ٲ��). �̷��� ��찡 �߻����� �ʵ��� ī�޶��� ��ġ ��
		���� y-���� �ּҰ��� (������ ���� + 5)�� �����Ѵ�. ī�޶��� ��ġ ������ y-���� �ּҰ��� ������ ��� ��ġ����
		ī�޶� ���� �Ʒ��� ��ġ���� �ʵ��� �����ؾ� �Ѵ�.*/
		CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
		float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) +
			5.0f;
		if (xmf3CameraPosition.y <= fHeight) {
			xmf3CameraPosition.y = fHeight;
			m_pCamera->SetPosition(xmf3CameraPosition);
			if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA) {
				//3��Ī ī�޶��� ��� ī�޶� ��ġ(y-��ǥ)�� ����Ǿ����Ƿ� ī�޶� �÷��̾ �ٶ󺸵��� �Ѵ�.
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