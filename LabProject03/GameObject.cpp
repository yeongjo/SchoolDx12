#include "stdafx.h"
#include "Shader.h"
#include "GameObject.h"
#include "Camera.h"
#include "GameFramework.h"
#include "Mesh.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial() {
	m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

CMaterial::~CMaterial() {
	if (m_pShader) {
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CMaterial::SetShader(CShader *pShader) {
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pMaterial) m_pMaterial->Release();
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader)
{
	if (!m_pMaterial)
	{
		m_pMaterial = new CMaterial();
		m_pMaterial->AddRef();
	}
	if (m_pMaterial) m_pMaterial->SetShader(pShader);
}

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void CGameObject::SetMaterial(UINT nReflection)
{
	if (!m_pMaterial) m_pMaterial = new CMaterial();
	m_pMaterial->m_nReflection = nReflection;
}

void CGameObject::ReleaseUploadBuffers() {
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
void CGameObject::Animate(float fTimeElapsed) {
}
void CGameObject::OnPrepareRender() {
}
void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle) {
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis),
		XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
CRotatingObject::CRotatingObject(int nMeshes) : CGameObject() {
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}
CRotatingObject::~CRotatingObject() {
}
void CRotatingObject::Animate(float fTimeElapsed) {
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {
}
void CGameObject::ReleaseShaderVariables() {
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {
	//XMFLOAT4X4 xmf4x4World;
	//XMStoreFloat4x4(&m_pcb, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	////객체의 월드 변환 행렬을 루트 상수(32-비트 값)를 통하여 셰이더 변수(상수 버퍼)로 복사한다. 
	//pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances) {
	OnPrepareRender(); //Nothing
	//if (IsVisible(pCamera)) {
	if (m_pMaterial) {
		if (m_pMaterial->m_pShader) {
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		}
	}
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, nInstances);
	//}
}
//인스턴싱 정점 버퍼 뷰를 사용하여 메쉬를 렌더링한다. 
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView) {
	OnPrepareRender(); //Nothing
	//if (IsVisible(pCamera)) {
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, nInstances, d3dInstancingBufferView);
	//}
}

void CGameObject::SetPosition(float x, float y, float z) {
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}
void CGameObject::SetPosition(XMFLOAT3 xmf3Position) {
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
XMFLOAT3 CGameObject::GetPosition() {
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}
//게임 객체의 로컬 z-축 벡터를 반환한다. 
XMFLOAT3 CGameObject::GetLook() {
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
		m_xmf4x4World._33)));
}
//게임 객체의 로컬 y-축 벡터를 반환한다. 
XMFLOAT3 CGameObject::GetUp() {
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23)));
}
//게임 객체의 로컬 x-축 벡터를 반환한다. 
XMFLOAT3 CGameObject::GetRight() {
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13)));
}
//게임 객체를 로컬 x-축 방향으로 이동한다. 
void CGameObject::MoveStrafe(float fDistance) {
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 y-축 방향으로 이동한다. 
void CGameObject::MoveUp(float fDistance) {
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 로컬 z-축 방향으로 이동한다. 
void CGameObject::MoveForward(float fDistance) {
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}
//게임 객체를 주어진 각도로 회전한다. 
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll) {
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch),
		XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
void CGameObject::Scale(float x, float y, float z) {
	auto s = XMMatrixScaling(x, y, z);
	m_xmf4x4World = Matrix4x4::Multiply(s, m_xmf4x4World);
}
bool CGameObject::IsVisible(CCamera *pCamera) {
	OnPrepareRender();
	bool bIsVisible = true;
	BoundingOrientedBox xmBoundingBox = m_pMesh->GetBoundingBox();
	//모델 좌표계의 바운딩 박스를 월드 좌표계로 변환한다. 
	xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingBox);
	return(bIsVisible);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList
	*pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int
	nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4
	xmf4Color) : CGameObject() {
	//지형에 사용할 높이 맵의 가로, 세로의 크기이다. 
	m_nWidth = nWidth;
	m_nLength = nLength;
	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크기이다. cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	//xmf3Scale는 지형을 실제로 몇 배 확대할 것인가를 나타낸다. 
	m_xmf3Scale = xmf3Scale;
	//지형에 사용할 높이 맵을 생성한다. 
	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);
	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다.
	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다. 
	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++) {
		for (int x = 0, xStart = 0; x < cxBlocks; x++) {
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치(좌표)이다. 
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다. 
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart,
				zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(pHeightMapGridMesh);
		}
	}
	//지형을 렌더링하기 위한 셰이더를 생성한다. 
	CTerrainShader *pShader = new CTerrainShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CHeightMapTerrain::~CHeightMapTerrain(void) {
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
void CHeightMapTerrain::Animate(float fTimeElapsed) {
	auto player = CGameFramework::getInstance().m_pPlayer;
	auto x = m_xmf3Scale.x * m_nWidth*0.5f;
	auto z = m_xmf3Scale.z * m_nLength*0.5f;
	auto pos = GetPosition();
	auto direction = Vector3::Subtract(player->GetPosition(), pos);
	if (abs(direction.x) >= x) {
		MoveStrafe((direction.x > 0 ? x : -x) * 2);
	}
	if (abs(direction.z) >= z) {
		MoveForward((direction.z > 0 ? z : -z) * 2);
	}
}
void CHeightMapTerrain::Render(ID3D12GraphicsCommandList * pd3dCommandList, CCamera * pCamera) {
	CHeightMapTerrain::Animate(0);
	CGameObject::Render(pd3dCommandList, pCamera);
	auto t = m_xmf4x4World;
	auto x = m_xmf3Scale.x * m_nWidth;
	auto z = m_xmf3Scale.z * m_nLength;
	MoveForward(-z);
	MoveStrafe(-x);
	for (size_t _x = 0; _x < 3; _x++) {
		for (size_t _z = 0; _z < 3; _z++) {
			UpdateShaderVariables(pd3dCommandList);
			//if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
			//게임 객체가 포함하는 모든 메쉬를 렌더링한다.
			if (m_pMesh) m_pMesh->Render(pd3dCommandList);
			MoveForward(z);
		}
		MoveStrafe(x);
		MoveForward(z*-3);
	}
	m_xmf4x4World = t;
}
void CGameObject::GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4&
	xmf4x4View, XMFLOAT3 *pxmf3PickRayOrigin, XMFLOAT3 *pxmf3PickRayDirection) {
	XMFLOAT4X4 xmf4x4WorldView = Matrix4x4::Multiply(m_xmf4x4World, xmf4x4View);
	XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(xmf4x4WorldView);
	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	//카메라 좌표계의 원점을 모델 좌표계로 변환한다.
	*pxmf3PickRayOrigin = Vector3::TransformCoord(xmf3CameraOrigin, xmf4x4Inverse);
	//카메라 좌표계의 점(마우스 좌표를 역변환하여 구한 점)을 모델 좌표계로 변환한다. 
	*pxmf3PickRayDirection = Vector3::TransformCoord(xmf3PickPosition, xmf4x4Inverse);
	//광선의 방향 벡터를 구한다. 
	*pxmf3PickRayDirection = Vector3::Normalize(Vector3::Subtract(*pxmf3PickRayDirection,
		*pxmf3PickRayOrigin));
}
int CGameObject::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4&
	xmf4x4View, float *pfHitDistance) {
	int nIntersected = 0;
	if (m_pMesh) {
		XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;
		//모델 좌표계의 광선을 생성한다. 
		GenerateRayForPicking(xmf3PickPosition, xmf4x4View, &xmf3PickRayOrigin,
			&xmf3PickRayDirection);
		//모델 좌표계의 광선과 메쉬의 교차를 검사한다. 
		nIntersected = m_pMesh->CheckRayIntersection(xmf3PickRayOrigin,
			xmf3PickRayDirection, pfHitDistance);
	}
	return(nIntersected);
}

void CMovingObject::Animate(float fTimeElapsed) {
	elapsedTime += fTimeElapsed;
	if (8 < elapsedTime) {
		elapsedTime = 0;
	}
	if (followObj) {
		auto followObjPos = XMVectorSet(followObj->m_xmf4x4World._41, followObj->m_xmf4x4World._42, followObj->m_xmf4x4World._43, 0);
		auto pos = XMVectorSet(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43, 0);
		auto direc = XMVector3Normalize(followObjPos - pos);
		auto direcVec = XMLoadFloat3(&Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
			m_xmf4x4World._33)));
		auto off = direc - direcVec;
		direcVec += off * fTimeElapsed * 5 * elapsedTime*elapsedTime;
		direcVec = XMVector3Normalize(direcVec);
		XMFLOAT3 m_xmf3MovingDirection;
		XMStoreFloat3(&m_xmf3MovingDirection, direcVec);
		m_xmf4x4World._31 = m_xmf3MovingDirection.x, m_xmf4x4World._32 = m_xmf3MovingDirection.y,
			m_xmf4x4World._33 = m_xmf3MovingDirection.z;
		XMFLOAT3 m_xmf3Up(0, 1, 0);
		XMFLOAT3 m_xmf3Right;
		m_xmf3MovingDirection = Vector3::Normalize(m_xmf3MovingDirection);
		m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3MovingDirection, true);
		m_xmf3Up = Vector3::CrossProduct(m_xmf3MovingDirection, m_xmf3Right, true);
		m_xmf4x4World._11 = m_xmf3Right.x, m_xmf4x4World._12 = m_xmf3Right.y,
			m_xmf4x4World._13 = m_xmf3Right.z;
		m_xmf4x4World._21 = m_xmf3Up.x, m_xmf4x4World._22 = m_xmf3Up.y,
			m_xmf4x4World._23 = m_xmf3Up.z;
	}
	CGameObject::MoveForward(m_fRotationSpeed * fTimeElapsed);
}

void CExplosibleObject::Animate(float fTimeElapsed) {
	if (isExploed) {
		if (firstExplosion) {
			firstExplosion = false;
			children.resize(50);
			for (size_t i = 0; i < 50; i++) {
				children[i].SetMesh(m_pMesh);
				for (size_t i = 0; i < children.size(); i++) {
					children[i].SetPosition(GetPosition());
				}
				children[i].Rotate(random()*180.0f, random()*180.0f, random()*180.0f);
				children[i].m_fRotationSpeed = 100;
			}
		}
		for (size_t i = 0; i < children.size(); i++) {
			children[i].Animate(fTimeElapsed);
		}
		elapsedTime += fTimeElapsed;
		if (elapsedTime > 2) {
			elapsedTime = 0;
			isExploed = false;
			for (size_t i = 0; i < children.size(); i++) {
				children[i].SetPosition(GetPosition());
			}
		}
		return;
	}
	CRotatingObject::Animate(fTimeElapsed);
	SetPosition(Vector3::Add(GetPosition(), direction));

	auto pos = CMapObject::getInstance().GetPosition();
	auto x = m_xmf4x4World._41;
	auto y = m_xmf4x4World._42;
	auto z = m_xmf4x4World._43;
	const float width = 900 / 2;
	if ((x < -width + pos.x && direction.x < 0) || (width + pos.x < x&& direction.x>0)) {
		direction.x = -direction.x;
		SetPosition(m_xmf4x4World._41 + direction.x * 2,
			m_xmf4x4World._42,
			m_xmf4x4World._43);
	}
	if ((y < -width + pos.y && direction.y < 0) || (width + pos.y < y&& direction.y>0)) {
		direction.y = -direction.y;
		SetPosition(m_xmf4x4World._41,
			m_xmf4x4World._42 + direction.y * 2,
			m_xmf4x4World._43);
	}
	if ((z < -width + pos.z && direction.z < 0) || (width + pos.z < z&& direction.z>0)) {
		direction.z = -direction.z;
		SetPosition(m_xmf4x4World._41,
			m_xmf4x4World._42,
			m_xmf4x4World._43 + direction.z * 2);
	}
}

void CMapObject::Animate(float fTimeElapsed) {
	auto player = CGameFramework::getInstance().m_pPlayer;
	auto width = 900 / 2;
	auto direction = Vector3::Subtract(player->GetPosition(), GetPosition());
	auto distance = Vector3::Length(direction);
	if (distance > width) {
		SetPosition(player->GetPosition());
	}
}


