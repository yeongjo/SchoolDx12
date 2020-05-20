#include "stdafx.h"
#include "Camera.h"
#include "Mesh.h"


CPoint3D CCamera::CameraTransform(CPoint3D& f3World)
{
	//카메라를 월드 좌표계의 원점으로 이동한다. 
	CPoint3D f3Camera = f3World;
	XMFLOAT4X4 transFloat4x4;
	XMStoreFloat4x4(&transFloat4x4, XMMatrixIdentity());
	transFloat4x4._41 -= m_fxPosition;
	transFloat4x4._42 -= m_fyPosition;
	transFloat4x4._43 -= m_fzPosition;

	float fPitch = DegreeToRadian(m_fxRotation);
	float fYaw = DegreeToRadian(m_fyRotation);
	float fRoll = DegreeToRadian(m_fzRotation);
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(fPitch, fYaw, fRoll);

	XMMATRIX transMat = XMLoadFloat4x4(&transFloat4x4);
	XMVECTOR pos = XMLoadFloat3(&f3Camera.pos);

	XMVECTOR t = XMVector3Transform(pos, rotMat*transMat);
	XMStoreFloat3(&f3Camera.pos, t);
	return f3Camera;
}


CPoint3D CCamera::ProjectionTransform(CPoint3D& f3Camera)
{
	CPoint3D f3Project = f3Camera;
	if (f3Camera.pos.z != 0.0f)
	{
		XMFLOAT4X4 transFloat4x4;
		XMStoreFloat4x4(&transFloat4x4, XMMatrixIdentity());
		transFloat4x4._34 = 1;
		transFloat4x4._44 = 0;
		transFloat4x4._11 = m_fProjectRectDistance;
		transFloat4x4._22 = m_fProjectRectDistance;
		XMMATRIX transMat = XMLoadFloat4x4(&transFloat4x4);

		XMVECTOR pos = XMLoadFloat3(&f3Project.pos);
		XMVECTOR t = XMVector4Transform(pos, transMat);
		t = t / XMVectorGetW(t);
		XMFLOAT3 t_pos;
		XMStoreFloat3(&t_pos, t);

		t_pos.x /= m_fAspectRatio;
		f3Project.pos = t_pos;

		////카메라의 시야각이 90°가 아닌 경우 투영 사각형까지의 거리를 곱한다. 
		//f3Project.pos.x = float((f3Camera.pos.x * m_fProjectRectDistance) / 
		//(m_fAspectRatio * f3Camera.pos.z));
		//f3Project.pos.y = float((f3Camera.pos.y * m_fProjectRectDistance) /
		//	f3Camera.pos.z);
		////투영 좌표계는 2차원이므로 z-좌표에 카메라 좌표계 z-좌표를 저장한다. 
		//f3Project.pos.z = f3Camera.pos.z;
	}
	return(f3Project);
}

CPoint3D CCamera::ScreenTransform(CPoint3D& f3Projection)
{
	CPoint3D f3Screen = f3Projection;
	XMFLOAT4 tt(f3Screen.pos.x, f3Screen.pos.y, f3Screen.pos.z, 1);

	XMFLOAT4X4 transFloat4x4;
	XMStoreFloat4x4(&transFloat4x4, XMMatrixIdentity());

	float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;

	transFloat4x4._11 = fHalfWidth;
	transFloat4x4._41 = m_pViewport->m_nLeft + fHalfWidth;
	transFloat4x4._22 = -fHalfHeight;
	transFloat4x4._42 = m_pViewport->m_nTop + fHalfHeight;
	XMMATRIX transMat = XMLoadFloat4x4(&transFloat4x4);

	XMVECTOR pos = XMLoadFloat4(&tt);
	XMVECTOR t = XMVector4Transform(pos, transMat);
	XMStoreFloat3(&f3Screen.pos, t);

	/*float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;
	f3Screen.pos.x = (f3Projection.pos.x * fHalfWidth) + m_pViewport->m_nLeft
		+ fHalfWidth;
	f3Screen.pos.y = (-f3Projection.pos.y * fHalfHeight) +
		m_pViewport->m_nTop + fHalfHeight;*/
	return(f3Screen);
}

void CCamera::SetViewport(int nLeft, int nTop, int nWidth, int
	nHeight)
{
	m_pViewport = new CViewport(nLeft, nTop, nWidth, nHeight);
	m_fAspectRatio = float(nWidth) / float(nHeight);
}
void CCamera::SetFOVAngle(float fFOVAngle)
{
	m_fFOVAngle = fFOVAngle;
	m_fProjectRectDistance = float(1.0f /
		tan(DegreeToRadian(fFOVAngle * 0.5f)));
}
void CCamera::Move(float x, float y, float z)
{
	m_fxPosition += x;
	m_fyPosition += y;
	m_fzPosition += z;
}
void CCamera::Rotate(float fPitch, float fYaw, float fRoll)
{
	m_fxRotation += fPitch;
	m_fyRotation += fYaw;
	m_fzRotation += fRoll;
}