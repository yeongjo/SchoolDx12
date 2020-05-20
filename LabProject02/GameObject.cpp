#include "stdafx.h"
#include "GameObject.h"

CGameObject::~CGameObject(void)
{
	//이 게임 객체는 더 이상 메쉬를 참조하지 않으므로 메쉬의 참조값을 1 감소한다.
	if (m_pMesh) m_pMesh->Release();
}

CPoint3D CGameObject::WorldTransform(CPoint3D& f3Model)
{
	CPoint3D f3World = f3Model;

	XMFLOAT4X4 locationFloat4x4;
	XMStoreFloat4x4(&locationFloat4x4, XMMatrixIdentity());
	locationFloat4x4._41 += m_fxPosition;
	locationFloat4x4._42 += m_fyPosition;
	locationFloat4x4._43 += m_fzPosition;

	float fPitch = DegreeToRadian(m_fxRotation);
	float fYaw = DegreeToRadian(m_fyRotation);
	float fRoll = DegreeToRadian(m_fzRotation);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(fPitch, fYaw, fRoll);
	XMMATRIX locMat = XMLoadFloat4x4(&locationFloat4x4);
	XMVECTOR pos = XMLoadFloat3(&f3World.pos);

	XMVECTOR t = XMVector3Transform(pos, rotMat*locMat);
	XMStoreFloat3(&f3World.pos, t);
	return f3World;


	//float fPitch = DegreeToRadian(m_fxRotation);
	//float fYaw = DegreeToRadian(m_fyRotation);
	//float fRoll = DegreeToRadian(m_fzRotation);

	//CPoint3D f3World = f3Model;
	//CPoint3D f3Rotated = f3Model;
	////회전 변환
	//if (fPitch != 0.0f)
	//{
	//	f3Rotated.pos.y = float(f3World.pos.y * cos(fPitch) - f3World.pos.z *
	//		sin(fPitch));
	//	f3Rotated.pos.z = float(f3World.pos.y * sin(fPitch) + f3World.pos.z *
	//		cos(fPitch));
	//	f3World.pos.y = f3Rotated.pos.y;
	//	f3World.pos.z = f3Rotated.pos.z;
	//}
	//if (fYaw != 0.0f)
	//{
	//	f3Rotated.pos.x = float(f3World.pos.x * cos(fYaw) + f3World.pos.z *
	//		sin(fYaw));
	//	f3Rotated.pos.z = float(-f3World.pos.x * sin(fYaw) + f3World.pos.z *
	//		cos(fYaw));
	//	f3World.pos.x = f3Rotated.pos.x;
	//	f3World.pos.z = f3Rotated.pos.z;
	//}
	//if (fRoll != 0.0f)
	//{
	//	f3Rotated.pos.x = float(f3World.pos.x * cos(fRoll) - f3World.pos.y *
	//		sin(fRoll));
	//	f3Rotated.pos.y = float(f3World.pos.x * sin(fRoll) + f3World.pos.y *
	//		cos(fRoll));
	//	f3World.pos.x = f3Rotated.pos.x;
	//	f3World.pos.y = f3Rotated.pos.y;
	//}
	////평행 이동 변환
	//f3World.pos.x += m_fxPosition;
	//f3World.pos.y += m_fyPosition;
	//f3World.pos.z += m_fzPosition;
	//return(f3World);
}

void CGameObject::Animate(float fElapsedTime)
{
	Rotate(m_fxRotationSpeed * fElapsedTime, m_fyRotationSpeed *
		fElapsedTime, m_fzRotationSpeed * fElapsedTime);
}

void CGameObject::Render(HDC hDCFrameBuffer)
{
	if (m_pMesh) {
		HPEN hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		m_pMesh->Render(hDCFrameBuffer);
		::SelectObject(hDCFrameBuffer, hOldPen);
		::DeleteObject(hPen);
	}
}
