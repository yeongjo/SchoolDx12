﻿#pragma once
#include "Mesh.h"
class CShader;
class CCamera;

class CGameObject {
public:
	CGameObject(int nMeshes = 1);
	virtual ~CGameObject();
private:
	int m_nReferences = 0;
public:
	void AddRef() {
		m_nReferences++;
	}
	void Release() {
		if (--m_nReferences <= 0) delete this;
	}
protected:
	CShader *m_pShader = NULL;
	//게임 객체는 여러 개의 메쉬를 포함하는 경우 게임 객체가 가지는 메쉬들에 대한 포인터와 그 개수이다. 
	CMesh *m_pMesh = NULL;
public:
	//게임 객체가 카메라에 보인는 가를 검사한다. 
	virtual bool IsVisible(CCamera *pCamera = NULL);
public:
	XMFLOAT4X4 m_xmf4x4World;
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void ReleaseUploadBuffers();
	void SetMesh(CMesh *pMesh);
	virtual void SetShader(CShader *pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances=1);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT
		nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView);
public:
	//상수 버퍼를 생성한다. 
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다. 
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다. 
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//게임 객체의 위치를 설정한다. 
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//게임 객체를 회전(x-축, y-축, z-축)한다. 
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Scale(float x, float y, float z);
public:
	//모델 좌표계의 픽킹 광선을 생성한다. 
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View,
		XMFLOAT3 *pxmf3PickRayOrigin, XMFLOAT3 *pxmf3PickRayDirection);
	//카메라 좌표계의 한 점에 대한 모델 좌표계의 픽킹 광선을 생성하고 객체와의 교차를 검사한다. 
	virtual int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfHitDistance);
	virtual float GetDistance(const XMFLOAT3& position) {
		return Vector3::Length(Vector3::Subtract(position, GetPosition()));
	}
};

class CRotatingObject : public CGameObject {
public:
	CRotatingObject(int nMeshes = 1);
	virtual ~CRotatingObject();
protected:
	XMFLOAT3 m_xmf3RotationAxis;
public:
	float m_fRotationSpeed;

	void SetRotationSpeed(float fRotationSpeed) {
		m_fRotationSpeed = fRotationSpeed;
	}
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) {
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}
	virtual void Animate(float fTimeElapsed);
};

class CMovingObject : public CRotatingObject {
public:
	float elapsedTime = 0;

	CGameObject* followObj = NULL;
	CMovingObject() :CRotatingObject() {
	}
	virtual ~CMovingObject() {
	}

	virtual void Animate(float fTimeElapsed);
};

class CHeightMapTerrain : public CGameObject {
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
		*pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int
		nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4
		xmf4Color);
	virtual ~CHeightMapTerrain();
	virtual void Animate(float fTimeElapsed);
private:
	//지형의 높이 맵으로 사용할 이미지이다. 
	CHeightMapImage *m_pHeightMapImage;
	//높이 맵의 가로와 세로 크기이다. 
	int m_nWidth;
	int m_nLength;
	//지형을 실제로 몇 배 확대할 것인가를 나타내는 스케일 벡터이다. 
	XMFLOAT3 m_xmf3Scale;

public:
	//지형의 높이를 계산하는 함수이다(월드 좌표계). 높이 맵의 높이에 스케일의 y를 곱한 값이다. 
	float GetHeight(float x, float z) {
		auto t = GetPosition();
		return(m_pHeightMapImage->GetHeight((x - t.x) / m_xmf3Scale.x , (z - t.z) / m_xmf3Scale.z )  * m_xmf3Scale.y + t.y);
	}
	//지형의 법선 벡터를 계산하는 함수이다(월드 좌표계). 높이 맵의 법선 벡터를 사용한다. 
	XMFLOAT3 GetNormal(float x, float z) {
		auto t = GetPosition();
		return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x - t.x), int(z / m_xmf3Scale.z - t.z)));
	}
	int GetHeightMapWidth() {
		return(m_pHeightMapImage->GetHeightMapWidth());
	}
	int GetHeightMapLength() {
		return(m_pHeightMapImage->GetHeightMapLength());
	}
	XMFLOAT3 GetScale() {
		return(m_xmf3Scale);
	}
	//지형의 크기(가로/세로)를 반환한다. 높이 맵의 크기에 스케일을 곱한 값이다. 
	float GetWidth() {
		return(m_nWidth * m_xmf3Scale.x);
	}
	float GetLength() {
		return(m_nLength * m_xmf3Scale.z);
	}
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual bool IsVisible(CCamera *pCamera = NULL) {
		return true;
	}
};

class CExplosibleObject : public CRotatingObject {
public:
	bool isExploed = false;
	float elapsedTime = 0;
	vector<CMovingObject> children;
	bool firstExplosion = true;
	XMFLOAT3 direction;
	CExplosibleObject(CMesh* mesh) {
		m_pMesh = mesh;
		direction = XMFLOAT3(random()*0.2f, random()*0.2f, random()*0.2f);
	}
	virtual ~CExplosibleObject() {
	}

	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {
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
	virtual void Animate(float fTimeElapsed);
	virtual int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float *pfHitDistance) {
		return 0;
	}
	virtual float GetDistance(const XMFLOAT3& position) {
		return FLT_MAX;
	}
	virtual bool IsVisible(CCamera *pCamera = NULL) {
		return true;
	}
};