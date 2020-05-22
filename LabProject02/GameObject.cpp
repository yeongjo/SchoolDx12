#include "stdafx.h"
#include "GameObject.h"
#include "Camera.h"
#include "GameFramework.h"
#include "GraphicsPipeline.h"
#include "Mesh.h"

CGameObject::~CGameObject(void){
	//이 게임 객체는 더 이상 메쉬를 참조하지 않으므로 메쉬의 참조값을 1 감소한다.
	if(m_pMesh)
		m_pMesh->Release();
}

void CGameObject::SetPosition(float x, float y, float z){
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}
void CGameObject::SetPosition(const XMFLOAT3 &xmf3Position){
	m_xmf4x4World._41 = xmf3Position.x;
	m_xmf4x4World._42 = xmf3Position.y;
	m_xmf4x4World._43 = xmf3Position.z;
}

void CGameObject::SetMovingDirection(const XMFLOAT3 &xmf3MovingDirection){
	XMStoreFloat3(&m_xmf3MovingDirection,
		XMVector3Normalize(XMLoadFloat3(&xmf3MovingDirection)));
}

void CGameObject::SetRotationAxis(const XMFLOAT3 &xmf3RotationAxis){
	XMStoreFloat3(&m_xmf3RotationAxis,
		XMVector3Normalize(XMLoadFloat3(&xmf3RotationAxis)));
}

//오일러 각도의 회전, 왼손좌표계에서 회전(자전) 행렬은 평행이동 행렬 왼쪽에
//곱해야 한다.
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll){
	XMMATRIX xmmtxRotate = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(fPitch), XMConvertToRadians(fYaw),
		XMConvertToRadians(fRoll));
	XMStoreFloat4x4(
		&m_xmf4x4World,
		XMMatrixMultiply(xmmtxRotate, XMLoadFloat4x4(&m_xmf4x4World)));
}

//회전축을 중심으로 회전, 왼손좌표계에서 회전(자전) 행렬은 평행이동 행렬 왼쪽에
//곱해야 한다.
void CGameObject::Rotate(const XMFLOAT3 &xmf3RotationAxis, float fAngle){
	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotationAxis),
		XMConvertToRadians(fAngle));
	XMStoreFloat4x4(
		&m_xmf4x4World,
		XMMatrixMultiply(xmmtxRotate, XMLoadFloat4x4(&m_xmf4x4World)));
}

void CGameObject::Move(const XMFLOAT3 &vDirection, float fSpeed){
	SetPosition(m_xmf4x4World._41+vDirection.x * fSpeed,
		m_xmf4x4World._42+vDirection.y * fSpeed,
		m_xmf4x4World._43+vDirection.z * fSpeed);
}
void CGameObject::Animate(float fElapsedTime){
	if(m_fRotationSpeed!=0.0f)
		Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);
	if(m_fMovingSpeed!=0.0f)
		Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera *pCamera){
	if(m_pMesh){
		CGraphicsPipeline::SetWorldTransform(&m_xmf4x4World);
		HPEN hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		m_pMesh->Render(hDCFrameBuffer);
		::SelectObject(hDCFrameBuffer, hOldPen);
		::DeleteObject(hPen);
	}
}

CGameObject *CGameObject::CheckCollision(const CGameObject *rhs){
	// 길이 구하기
	XMFLOAT3 diff;
	diff.x = m_xmf4x4World._41-rhs->m_xmf4x4World._41;
	diff.y = m_xmf4x4World._42-rhs->m_xmf4x4World._42;
	diff.z = m_xmf4x4World._43-rhs->m_xmf4x4World._43;
	auto diffVec = XMLoadFloat3(&diff);
	auto len = XMVector3LengthSq(diffVec);
	float distance = 0.0f;
	XMStoreFloat(&distance, len);

	float &&radi = radius+rhs->radius;
	if(radi<=0 || distance>(radi * radi))
		return nullptr;
	return this;
}

CGameObject *CGameObject::CheckCollision(XMFLOAT3 origin, XMFLOAT3 direc){
	if(radius<=0) return nullptr;
	XMFLOAT3 diff;
	diff.x = m_xmf4x4World._41;
	diff.y = m_xmf4x4World._42;
	diff.z = m_xmf4x4World._43;
	float dist;
	auto test = XMLoadFloat3(&origin)-XMLoadFloat3(&direc)*2;
	if(BoundingSphere(diff, radius)
		.Intersects(test, XMLoadFloat3(&direc), dist)){
		return this;
	}
	return nullptr;
}

CExplosedObjects::CExplosedObjects(){
	auto mesh = new CCubeMesh();
	mainObj.SetMesh(mesh);
	XMFLOAT3 direc(random(), random(), random());
	normalize(direc);
	mainObj.SetMovingDirection(direc);
	mainObj.SetRotationAxis(direc);
	mainObj.SetRotationSpeed(30);
	for(size_t i = 0; i<50; i++){
		objs[i].SetMesh(mesh);
		XMFLOAT3 direc(random(), random(), random());
		normalize(direc);
		objs[i].SetMovingDirection(direc);
		objs[i].SetMovingSpeed(30);
		objs[i].SetColor(m_dwColor);
	}
}

void CExplosedObjects::Animate(float fElapsedTime){

	// 비활성화는 폭발상태
	if(!m_bActive){
		if(!isExplode) // 한번만 초기 위치 설정함
			for(size_t i = 0; i<50; i++){
				objs[i].SetPosition(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
			}
		isExplode = true;
		elapsedExplodeTime += fElapsedTime;
		if(explodeRestoreTime<elapsedExplodeTime){
			elapsedExplodeTime = 0;
			m_bActive = true;
			isExplode = false;
		}
		for(auto &t:objs)
			t.Animate(fElapsedTime);
	} else{
		CGameObject::Animate(fElapsedTime);
	}
}

void CExplosedObjects::Render(HDC hDCFrameBuffer, CCamera *pCamera){
	if(!m_bActive){
		for(auto &t:objs)
			t.Render(hDCFrameBuffer, pCamera);
	} else{
		CGameObject::Render(hDCFrameBuffer, pCamera);
	}
}

CBullet::CBullet(XMFLOAT3 direc){
	SetMovingDirection(direc);
	SetMovingSpeed(30);
}

void CBullet::Animate(float fElapsedTime){
	if(!m_bActive)
		return;

	// 충돌검사
	auto game = CGameFramework::getInstance();
	auto collobj = game.CheckCollision(this);
	if(collobj!=nullptr){
		m_bActive = collobj->m_bActive = false;
		target = nullptr;
		elapsedTime = 0;
	}

	elapsedTime += fElapsedTime;
	if(destoryTime<elapsedTime){
		m_bActive = false;
		target = nullptr;
		elapsedTime = 0;
	}

	if(target!=nullptr){
		auto targetPos = XMVectorSet(target->m_xmf4x4World._41, target->m_xmf4x4World._42, target->m_xmf4x4World._43, 0);
		auto pos = XMVectorSet(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43, 0);
		auto direc = XMVector3Normalize(targetPos-pos);
		auto direcVec = XMLoadFloat3(&m_xmf3MovingDirection);
		auto off = direc-direcVec;
		direcVec+=off*fElapsedTime*5*elapsedTime*elapsedTime;
		direcVec = XMVector3Normalize(direcVec);
		XMStoreFloat3(&m_xmf3MovingDirection, direcVec);
	}
	CGameObject::Animate(fElapsedTime);
}
