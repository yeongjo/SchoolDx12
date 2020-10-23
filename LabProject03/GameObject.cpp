#include "stdafx.h"
#include "Shader.h"
#include "GameObject.h"
#include "Camera.h"
#include "GameFramework.h"
#include "Mesh.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers, int nRootParameters) {
	m_nTextureType = nTextureType;

	m_nTextures = nTextures;
	if (m_nTextures > 0) {
		m_ppd3dTextureUploadBuffers = new ID3D12Resource *[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource *[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_ppd3dTextureUploadBuffers[i] = m_ppd3dTextures[i] = nullptr;

		m_ppstrTextureNames = new _TCHAR[m_nTextures][64];
		for (int i = 0; i < m_nTextures; i++) m_ppstrTextureNames[i][0] = '\0';

		m_pd3dSrvGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nTextures];
		for (int i = 0; i < m_nTextures; i++) m_pd3dSrvGpuDescriptorHandles[i].ptr = NULL;

		m_pnResourceTypes = new UINT[m_nTextures];
		m_pdxgiBufferFormats = new DXGI_FORMAT[m_nTextures];
		m_pnBufferElements = new int[m_nTextures];
	}
	m_nRootParameters = nRootParameters;
	if (nRootParameters > 0) m_pnRootParameterIndices = new int[nRootParameters];
	for (int i = 0; i < m_nRootParameters; i++) m_pnRootParameterIndices[i] = -1;

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture() {
	if (m_ppd3dTextures) {
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextures[i]) m_ppd3dTextures[i]->Release();
		delete[] m_ppd3dTextures;
	}

	if (m_ppstrTextureNames) delete[] m_ppstrTextureNames;

	if (m_pnResourceTypes) delete[] m_pnResourceTypes;
	if (m_pdxgiBufferFormats) delete[] m_pdxgiBufferFormats;
	if (m_pnBufferElements) delete[] m_pnBufferElements;

	if (m_pnRootParameterIndices) delete[] m_pnRootParameterIndices;
	if (m_pd3dSrvGpuDescriptorHandles) delete[] m_pd3dSrvGpuDescriptorHandles;

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootParameterIndex(int nIndex, UINT nRootParameterIndex) {
	m_pnRootParameterIndices[nIndex] = nRootParameterIndex;
}

void CTexture::SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle) {
	m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle) {
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {
	if (m_nRootParameters == m_nTextures) {
		for (int i = 0; i < m_nRootParameters; i++) {
			if (m_pd3dSrvGpuDescriptorHandles[i].ptr && (m_pnRootParameterIndices[i] != -1))
				pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[i], m_pd3dSrvGpuDescriptorHandles[i]);
		}
	} else {
		if (m_pd3dSrvGpuDescriptorHandles[0].ptr)
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[0], m_pd3dSrvGpuDescriptorHandles[0]);
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex) {
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pnRootParameterIndices[nParameterIndex], m_pd3dSrvGpuDescriptorHandles[nTextureIndex]);
}

void CTexture::ReleaseShaderVariables() {
}

void CTexture::ReleaseUploadBuffers() {
	if (m_ppd3dTextureUploadBuffers) {
		for (int i = 0; i < m_nTextures; i++) if (m_ppd3dTextureUploadBuffers[i]) m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = nullptr;
	}
}

void CTexture::LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex) {
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromDDSFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_GENERIC_READ/*D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE*/);
}

void CTexture::LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex) {
	m_pnResourceTypes[nIndex] = RESOURCE_BUFFER;
	m_pdxgiBufferFormats[nIndex] = ndxgiFormat;
	m_pnBufferElements[nIndex] = nElements;
	m_ppd3dTextures[nIndex] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pData, nElements * nStride, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_GENERIC_READ, &m_ppd3dTextureUploadBuffers[nIndex]);
}

ID3D12Resource* CTexture::CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue) {
	m_pnResourceTypes[nIndex] = nResourceType;
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, nElements, nMipLevels, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

int CTexture::LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader, UINT nIndex) {
	char pstrTextureName[64] = { '\0' };

	BYTE nStrLength = 64;
	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(pstrTextureName, sizeof(char), nStrLength, pInFile);
	pstrTextureName[nStrLength] = '\0';

	bool bDuplicated = false;
	bool bLoaded = false;
	if (strcmp(pstrTextureName, "null")) {
		bLoaded = true;
		char pstrFilePath[64] = { '\0' };
		strcpy_s(pstrFilePath, 64, ASSETPATH"/Model/Textures/");
		auto len = strlen(pstrFilePath);
		bDuplicated = (pstrTextureName[0] == '@');
		strcpy_s(pstrFilePath + len, 64 - len, (bDuplicated) ? (pstrTextureName + 1) : pstrTextureName);
		auto fileNameLen = ((bDuplicated) ? (nStrLength - 1) : nStrLength);
		strcpy_s(pstrFilePath + len + fileNameLen, 64 - len - fileNameLen, ".dds");

		size_t nConverted = 0;
		mbstowcs_s(&nConverted, m_ppstrTextureNames[nIndex], 64, pstrFilePath, _TRUNCATE);

#define _WITH_DISPLAY_TEXTURE_NAME

#ifdef _WITH_DISPLAY_TEXTURE_NAME
		static int nTextures = 0, nRepeatedTextures = 0;
		TCHAR pstrDebug[256] = { 0 };
		_stprintf_s(pstrDebug, 256, _T("Texture Name: %d %c %s\n"), (pstrTextureName[0] == '@') ? nRepeatedTextures++ : nTextures++, (pstrTextureName[0] == '@') ? '@' : ' ', m_ppstrTextureNames[nIndex]);
		OutputDebugString(pstrDebug);
#endif
		if (!bDuplicated) {
			LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, m_ppstrTextureNames[nIndex], RESOURCE_TEXTURE2D, nIndex);
			pShader->CreateShaderResourceView(pd3dDevice, this, nIndex);
#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
			m_pnRootParameterIndices[nIndex] = PARAMETER_STANDARD_TEXTURE + nIndex;
#endif
		} else {
			if (pParent) {
				CGameObject* pRootGameObject = pParent;
				while (pRootGameObject) {
					if (!pRootGameObject->m_pParent) break;
					pRootGameObject = pRootGameObject->m_pParent;
				}
				D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle;
				int nParameterIndex = pRootGameObject->FindReplicatedTexture(m_ppstrTextureNames[nIndex], &d3dSrvGpuDescriptorHandle);
				if (nParameterIndex >= 0) {
					m_pd3dSrvGpuDescriptorHandles[nIndex] = d3dSrvGpuDescriptorHandle;
					m_pnRootParameterIndices[nIndex] = nParameterIndex;
				}
			}
		}
	}
	return(bLoaded);
}

D3D12_SHADER_RESOURCE_VIEW_DESC CTexture::GetShaderResourceViewDesc(int nIndex) {
	ID3D12Resource* pShaderResource = GetResource(nIndex);
	D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	int nTextureType = GetTextureType(nIndex);
	switch (nTextureType) {
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY: //[]
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = 1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.Format = m_pdxgiBufferFormats[nIndex];
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = m_pnBufferElements[nIndex];
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial() {
}

CMaterial::~CMaterial() {
	if (m_pTexture) m_pTexture->Release();
	if (m_pShader) m_pShader->Release();
}

void CMaterial::SetShader(CShader *pShader) {
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial::SetTexture(CTexture* pTexture) {
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CMaterial::ReleaseUploadBuffers() {
	//	if (m_pShader) m_pShader->ReleaseUploadBuffers();
	if (m_pTexture) m_pTexture->ReleaseUploadBuffers();
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AmbientColor, 16);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4AlbedoColor, 20);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4SpecularColor, 24);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 4, &m_xmf4EmissiveColor, 28);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 1, &m_nType, 32);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables() {
	if (m_pShader) m_pShader->ReleaseShaderVariables();
	if (m_pTexture) m_pTexture->ReleaseShaderVariables();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject::CGameObject() {
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::CGameObject(int nMaterials) : CGameObject() {
	m_nMaterials = nMaterials;
	if (m_nMaterials > 0) {
		m_ppMaterials = new CMaterial*[m_nMaterials];
		for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = nullptr;
	}
}

CGameObject::~CGameObject() {
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0) {
		for (int i = 0; i < m_nMaterials; i++) {
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;
}

void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader *pShader) {
	if (!m_nMaterials) {
		m_nMaterials = 1;
		m_ppMaterials = new CMaterial *[m_nMaterials];
		m_ppMaterials[0] = new CMaterial();
	}
	m_ppMaterials[0]->SetShader(pShader);
}

void CGameObject::SetShader(int nMaterial, CShader *pShader) {
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject::SetMaterial(int nMaterial, CMaterial *pMaterial) {
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}
void CGameObject::ReleaseUploadBuffers() {
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
void CGameObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}
void CGameObject::OnPrepareRender() {
}
void CGameObject::SetChild(CGameObject *pChild) {
	if (m_pChild) {
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	} else {
		m_pChild = pChild;
	}
	if (pChild) {
		pChild->m_pParent = this;
	}
}
void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) {
}
void CGameObject::ReleaseShaderVariables() {
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) {
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {
	//OnPrepareRender();

	UpdateShaderVariables(pd3dCommandList);

	if (m_nMaterials > 0) {
		for (int i = 0; i < m_nMaterials; i++) {
			if (m_ppMaterials[i]) {
				if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
				m_ppMaterials[i]->UpdateShaderVariables(pd3dCommandList);
			}

			if (m_pMesh) m_pMesh->Render(pd3dCommandList, i);
		}
	}
	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances) {
	OnPrepareRender(); //Nothing
	//if (IsVisible(pCamera)) {
	//if (m_pMaterial) {
	//	if (m_pMaterial->m_pShader) {
	//		m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
	//		m_pMaterial->m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World); // CObjectsShader::UpdateShaderVariables에서 해줌
	//	}
	//}
	if (m_pMesh) m_pMesh->Render(pd3dCommandList, nInstances);
	//}
}
//인스턴싱 정점 버퍼 뷰를 사용하여 메쉬를 렌더링한다. 
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, D3D12_VERTEX_BUFFER_VIEW d3dInstancingBufferView) {
	OnPrepareRender(); //Nothing
	//if (IsVisible(pCamera)) {
	//if (m_pMesh) m_pMesh->Render(pd3dCommandList, nInstances, d3dInstancingBufferView);
	//}
}

void CGameObject::SetPosition(float x, float y, float z) {
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;
	UpdateTransform(nullptr);
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
CGameObject * CGameObject::AddSibling(CGameObject * obj) {
	return obj;
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
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
	UpdateTransform(nullptr);
}
void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle) {
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis),
		XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
	UpdateTransform(nullptr);
}
void CGameObject::Scale(float x, float y, float z) {
	auto s = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(s, m_xmf4x4Transform);
	UpdateTransform(nullptr);
}
bool CGameObject::IsVisible(CCamera *pCamera) {
	return true;
	//OnPrepareRender();
	//bool bIsVisible = true;
	//BoundingOrientedBox xmBoundingBox = m_pMesh->GetBoundingBox();
	////모델 좌표계의 바운딩 박스를 월드 좌표계로 변환한다. 
	//xmBoundingBox.Transform(xmBoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	//if (pCamera) bIsVisible = pCamera->IsInFrustum(xmBoundingBox);
	//return(bIsVisible);
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent) {
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

CGameObject *CGameObject::FindFrame(char *pstrFrameName) {
	CGameObject *pFrameObject = nullptr;
	if (!strncmp(m_pstrFrameName, pstrFrameName, strlen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(nullptr);
}

int CGameObject::FindReplicatedTexture(_TCHAR* pstrTextureName, D3D12_GPU_DESCRIPTOR_HANDLE* pd3dSrvGpuDescriptorHandle) {
	int nParameterIndex = -1;

	for (int i = 0; i < m_nMaterials; i++) {
		if (m_ppMaterials[i] && m_ppMaterials[i]->m_pTexture) {
			int nTextures = m_ppMaterials[i]->m_pTexture->GetTextures();
			for (int j = 0; j < nTextures; j++) {
				if (!_tcsncmp(m_ppMaterials[i]->m_pTexture->GetTextureName(j), pstrTextureName, _tcslen(pstrTextureName))) {
					*pd3dSrvGpuDescriptorHandle = m_ppMaterials[i]->m_pTexture->GetGpuDescriptorHandle(j);
					nParameterIndex = m_ppMaterials[i]->m_pTexture->GetRootParameter(j);
					return(nParameterIndex);
				}
			}
		}
	}
	if (m_pSibling) if ((nParameterIndex = m_pSibling->FindReplicatedTexture(pstrTextureName, pd3dSrvGpuDescriptorHandle)) > 0) return(nParameterIndex);
	if (m_pChild) if ((nParameterIndex = m_pChild->FindReplicatedTexture(pstrTextureName, pd3dSrvGpuDescriptorHandle)) > 0) return(nParameterIndex);

	return(nParameterIndex);
}

void CGameObject::LoadMaterialsFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CGameObject *pParent, FILE *pInFile, CShader *pShader) {
	char pstrToken[64] = { '\0' };

	int nMaterial = 0;
	BYTE nStrLength = 0;

	UINT nReads = (UINT)::fread(&m_nMaterials, sizeof(int), 1, pInFile);

	m_ppMaterials = new CMaterial*[m_nMaterials];
	for (int i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = nullptr;

	CMaterial *pMaterial = nullptr;
	CTexture* pTexture = nullptr;

	for (; ; ) {
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<Material>:")) {
			nReads = (UINT)::fread(&nMaterial, sizeof(int), 1, pInFile);

			pMaterial = new CMaterial();
#ifdef _WITH_STANDARD_TEXTURE_MULTIPLE_DESCRIPTORS
			pTexture = new CTexture(7, RESOURCE_TEXTURE2D, 0, 7); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
#else
			pTexture = new CTexture(7, RESOURCE_TEXTURE2D, 0, 1); //0:Albedo, 1:Specular, 2:Metallic, 3:Normal, 4:Emission, 5:DetailAlbedo, 6:DetailNormal
			pTexture->SetRootParameterIndex(0, PARAMETER_STANDARD_TEXTURE);
#endif
			pMaterial->SetTexture(pTexture);
			//			pMaterial->SetShader(pShader);
			SetMaterial(nMaterial, pMaterial);

			UINT nMeshType = GetMeshType();
			//if (nMeshType & VERTEXT_NORMAL_TEXTURE) pMaterial->SetStandardShader();
		} else if (!strcmp(pstrToken, "<AlbedoColor>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_xmf4AlbedoColor), sizeof(float), 4, pInFile);
		} else if (!strcmp(pstrToken, "<EmissiveColor>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_xmf4EmissiveColor), sizeof(float), 4, pInFile);
		} else if (!strcmp(pstrToken, "<SpecularColor>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_xmf4SpecularColor), sizeof(float), 4, pInFile);
		} else if (!strcmp(pstrToken, "<Glossiness>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_fGlossiness), sizeof(float), 1, pInFile);
		} else if (!strcmp(pstrToken, "<Smoothness>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_fSmoothness), sizeof(float), 1, pInFile);
		} else if (!strcmp(pstrToken, "<Metallic>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_fSpecularHighlight), sizeof(float), 1, pInFile);
		} else if (!strcmp(pstrToken, "<SpecularHighlight>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_fMetallic), sizeof(float), 1, pInFile);
		} else if (!strcmp(pstrToken, "<GlossyReflection>:")) {
			nReads = (UINT)::fread(&(pMaterial->m_fGlossyReflection), sizeof(float), 1, pInFile);
		} else if (!strcmp(pstrToken, "<AlbedoMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 0)) pMaterial->SetMaterialType(MATERIAL_ALBEDO_MAP);
		} else if (!strcmp(pstrToken, "<SpecularMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 1)) pMaterial->SetMaterialType(MATERIAL_SPECULAR_MAP);
		} else if (!strcmp(pstrToken, "<NormalMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 2)) pMaterial->SetMaterialType(MATERIAL_NORMAL_MAP);
		} else if (!strcmp(pstrToken, "<MetallicMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 3)) pMaterial->SetMaterialType(MATERIAL_METALLIC_MAP);
		} else if (!strcmp(pstrToken, "<EmissionMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 4)) pMaterial->SetMaterialType(MATERIAL_EMISSION_MAP);
		} else if (!strcmp(pstrToken, "<DetailAlbedoMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 5)) pMaterial->SetMaterialType(MATERIAL_DETAIL_ALBEDO_MAP);
		} else if (!strcmp(pstrToken, "<DetailNormalMap>:")) {
			if (pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader, 6)) pMaterial->SetMaterialType(MATERIAL_DETAIL_NORMAL_MAP);
		} else if (!strcmp(pstrToken, "</Materials>")) {
			break;
		}
	}
}

CGameObject *CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CGameObject *pParent, FILE *pInFile, CShader *pShader) {
	char pstrToken[64] = { '\0' };

	BYTE nStrLength = 0;
	UINT nReads = 0;

	int nFrame = 0, nTextures = 0;

	CGameObject *pGameObject = nullptr;

	for (; ; ) {
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<Frame>:")) {
			pGameObject = new CGameObject();

			nReads = (UINT)::fread(&nFrame, sizeof(int), 1, pInFile);
			nReads = (UINT)::fread(&nTextures, sizeof(int), 1, pInFile);

			nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
			nReads = (UINT)::fread(pGameObject->m_pstrFrameName, sizeof(char), nStrLength, pInFile);
			pGameObject->m_pstrFrameName[nStrLength] = '\0';
		} else if (!strcmp(pstrToken, "<Transform>:")) {
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			nReads = (UINT)::fread(&xmf3Position, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf3Rotation, sizeof(float), 3, pInFile); //Euler Angle
			nReads = (UINT)::fread(&xmf3Scale, sizeof(float), 3, pInFile);
			nReads = (UINT)::fread(&xmf4Rotation, sizeof(float), 4, pInFile); //Quaternion
		} else if (!strcmp(pstrToken, "<TransformMatrix>:")) {
			nReads = (UINT)::fread(&pGameObject->m_xmf4x4Transform, sizeof(float), 16, pInFile);
		} else if (!strcmp(pstrToken, "<Mesh>:")) {
			CStandardMesh *pMesh = new CStandardMesh(pd3dDevice, pd3dCommandList);
			pMesh->LoadMeshFromFile(pd3dDevice, pd3dCommandList, pInFile);
			pGameObject->SetMesh(pMesh);
		} else if (!strcmp(pstrToken, "<Materials>:")) {
			pGameObject->LoadMaterialsFromFile(pd3dDevice, pd3dCommandList, pParent, pInFile, pShader);
		} else if (!strcmp(pstrToken, "<Children>:")) {
			int nChilds = 0;
			nReads = (UINT)::fread(&nChilds, sizeof(int), 1, pInFile);
			if (nChilds > 0) {
				for (int i = 0; i < nChilds; i++) {
					CGameObject *pChild = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pGameObject, pInFile, pShader);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		} else if (!strcmp(pstrToken, "</Frame>")) {
			break;
		}
	}
	return(pGameObject);
}

void CGameObject::PrintFrameInfo(CGameObject *pGameObject, CGameObject *pParent) {
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

CGameObject *CGameObject::LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, char *pstrFileName, CShader *pShader) {
	FILE *pInFile = nullptr;
	::fopen_s(&pInFile, pstrFileName, "rb");
	::rewind(pInFile);

	CGameObject *pGameObject = CGameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, nullptr, pInFile, pShader);

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("Frame Hierarchy\n"));
	OutputDebugString(pstrDebug);

	CGameObject::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pGameObject);
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
		nIntersected = m_pMesh->GetIntersectRayCount(xmf3PickRayOrigin,
			xmf3PickRayDirection, pfHitDistance);
	}
	return(nIntersected);
}

inline float CGameObject::GetDistance(const XMFLOAT3 & position) {
	return Vector3::Length(Vector3::Subtract(position, GetPosition()));
}
//========================================================================
//

CSuperCobraObject::CSuperCobraObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) {
}

CSuperCobraObject::~CSuperCobraObject() {
}

void CSuperCobraObject::PrepareAnimate() {
	m_pMainRotorFrame = FindFrame("MainRotor");
	m_pTailRotorFrame = FindFrame("TailRotor");
}

void CSuperCobraObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	if (m_pMainRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGunshipObject::CGunshipObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) {
}

CGunshipObject::~CGunshipObject() {
}

void CGunshipObject::PrepareAnimate() {
	m_pMainRotorFrame = FindFrame("Rotor");
	m_pTailRotorFrame = FindFrame("Back_Rotor");
}

void CGunshipObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	if (m_pMainRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMi24Object::CMi24Object(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) {
}

CMi24Object::~CMi24Object() {
}

void CMi24Object::PrepareAnimate() {
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

void CMi24Object::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	if (m_pMainRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame) {
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(1) {
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 20.0f);
	SetMesh(pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE_CUBE, 0, 1);
	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, LASSETPATH L"/Model/Textures/SkyBox_0.dds", RESOURCE_TEXTURE_CUBE, 0);
	//	pSkyBoxTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"SkyBox/SkyBox_1.dds", RESOURCE_TEXTURE_CUBE, 0);

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList); // 없음

	pSkyBoxShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	pSkyBoxShader->CreateShaderResourceViews(pd3dDevice, pSkyBoxTexture, 0, PARAMETER_SKYBOX_CUBE_TEXTURE);

	CMaterial *pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);
	pSkyBoxMaterial->SetShader(pSkyBoxShader);

	SetMaterial(0, pSkyBoxMaterial);
}

CSkyBox::~CSkyBox() {
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	CGameObject::Render(pd3dCommandList, pCamera);
}
//========================================================================
//========================================================================
#ifdef HIGHTMAPTERRAINOBJ_ON
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject(1) {
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	UINT m_nMeshes = cxBlocks * czBlocks;
	auto m_ppMeshes = new CMesh*[m_nMeshes];
	for (UINT i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = nullptr;

	CHeightMapGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++) {
		for (int x = 0, xStart = 0; x < cxBlocks; x++) {
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			auto obj = new CGameObject(1);
			obj->SetMesh(pHeightMapGridMesh);
			SetChild(obj);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(5, RESOURCE_TEXTURE2D, 0, 1);

	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Base_Texture.dds", RESOURCE_TEXTURE2D, 0);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Detail_Texture_7.dds", RESOURCE_TEXTURE2D, 1);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Detail_Texture_1.dds", RESOURCE_TEXTURE2D, 2);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Lava(Diffuse).dds", RESOURCE_TEXTURE2D, 3);
	//	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/HeightMap-Alpha(Flipped).dds", RESOURCE_TEXTURE2D, 4);
	pTerrainTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/HeightMap2(Flipped)Alpha.dds", RESOURCE_TEXTURE2D, 4);

	//UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	//pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList); // Nothing
	pTerrainShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 5);
	//pTerrainShader->CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbGameObject, ncbElementBytes);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pTerrainTexture, 0, 11);

	CMaterial *pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);

	SetMaterial(0, pTerrainMaterial);

	//m_d3dCbvGPUDescriptorHandle = (pTerrainShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pTerrainShader);
}

CHeightMapTerrain::~CHeightMapTerrain(void) {
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}
//////////////////////////////////////////////////////////////////////
//
void CFollowObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	elapsedTime += fTimeElapsed;
	if (8 < elapsedTime) {
		elapsedTime = 0;
	}
	if (followObj) {
		auto followObjPos = XMVectorSet(followObj->m_xmf4x4World._41, followObj->m_xmf4x4World._42, followObj->m_xmf4x4World._43, 0);
		auto pos = XMVectorSet(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43, 0);

		auto offset = followObjPos - pos;
		float length;
		XMStoreFloat(&length, XMVector3Length(offset));
		if(length < speed * fTimeElapsed * 10) {
			// 도착
			if(OnArrived)
				OnArrived(this, followObj);
		}
		
		auto direc = XMVector3Normalize(offset);
		auto direcVec = XMLoadFloat3(&Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
			m_xmf4x4World._33)));
		auto off = direc - direcVec;
		direcVec += off * fTimeElapsed * speed * elapsedTime*elapsedTime;
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

void CExplosibleObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
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

void CMapObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	auto player = CGameFramework::getInstance().m_pPlayer;
	auto width = 900 / 2;
	auto direction = Vector3::Subtract(player->GetPosition(), GetPosition());
	auto distance = Vector3::Length(direction);
	if (distance > width) {
		SetPosition(player->GetPosition());
	}
}

void WayPointFollower::Destroy() {
	vector<WayPointFollower*> *objs;
	if (direc > 0) {
		objs = &objectSpawner->Objs0;
	} else {
		objs = &objectSpawner->Objs1;
	}
	for (size_t i = 0; i < objs->size(); i++) {
		if (objs[0][i] == this) {
			objs->erase(objs->begin() + i);
			break;
		}
	}
	isDestroyNextFrame = true;
}

void WayPointFollower::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent) {
	pos += direc * speed * fTimeElapsed * canMove;
	pos = clamp(pos, 0, 1);
	auto wayPointHandler = CGameFramework::getInstance().GetScene()->_wayPointHandler;
	auto t1 = wayPointHandler->getPosition(pos);
	auto t0 = wayPointHandler->getPosition(prevPos);
	SetPosition(Vector3::Add(t1,offset));
	if (pos != prevPos) {
		auto look = Vector3::Normalize(Vector3::Subtract(t1, t0));
		auto up = XMFLOAT3(0, 1, 0);
		auto right = Vector3::CrossProduct(up, look);

		m_xmf4x4Transform._31 = look.x, m_xmf4x4Transform._32 = look.y, m_xmf4x4Transform._33 = look.z;
		m_xmf4x4Transform._21 = up.x, m_xmf4x4Transform._22 = up.y, m_xmf4x4Transform._23 = up.z;
		m_xmf4x4Transform._11 = right.x, m_xmf4x4Transform._12 = right.y, m_xmf4x4Transform._13 = right.z;
		Scale(2, 2, 2);
	}

	vector<WayPointFollower*> *objs;
	// 적군의 정보 받기
	if(direc > 0) {
		// 아군쪽의 경우 objs1이 적군
		objs = &objectSpawner->Objs1;
	}else {
		objs = &objectSpawner->Objs0;
	}

	for (size_t i = 0; i < objs->size(); i++) {
		auto off = abs(pos - objs[0][i]->pos);
		if (attackRange >= off) {
			// 공격범위 안에 들어옴
			canMove = 0;
			attackDelay -= fTimeElapsed;
			if (attackDelay < 0) {
				attackDelay = Random(1.5f, 3.5f);
				// 딜레이가 지나 미사일 발사
				objectSpawner->AddBullet(this, objs[0][i]);
				//CBillboardParticleShader::AddParticle(GetPosition());
			}
			break;
		}
		canMove = 1;
	}

	CGameObject::Animate(fTimeElapsed, pxmf4x4Parent);

	prevPos = pos;
}

void ObjectSpawner::InitBullet(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {
	auto explosionTex = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	explosionTex->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/missile.dds", RESOURCE_TEXTURE2D, 0);
	auto bulletMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList);
	auto material = new CMaterial();
	material->SetTexture(explosionTex);
	bulletObj = new CFollowObject();
	bulletObj->SetMaterial(0, material);
	bulletObj->SetMesh(bulletMesh);
	bulletObj->Scale(.1f, 1, 0.5f);
	bulletObj->SetRotationSpeed(60);

	bulletShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 0, 1);
	bulletShader->CreateShaderResourceViews(pd3dDevice, explosionTex, 0, 17);
}

void ObjectSpawner::AddBullet(WayPointFollower* owner, WayPointFollower* target) {
	auto t = new CFollowObject(*bulletObj);
	bulletShader->m_ppObjects.push_back(t);
	t->SetPosition(owner->GetPosition());
	t->followObj = target;
	t->OnArrived = [](CFollowObject* self, CGameObject* target) {
		auto bulletShader = (CTexturedShader*)CGameFramework::getInstance().GetScene()->m_ppShaders[3];
		self->isDestroyNextFrame = true;

		auto follower = (WayPointFollower*)target;
		follower->durability -= 1;
		if(follower->durability <= 0) {
			// 죽음
			CBillboardParticleShader::AddParticle(follower->GetPosition());
			follower->Destroy();
		}else {
			// 피격
			CBillboardParticleShader::AddParticle(follower->GetPosition(), 0.3f, 1);
		}
	};
}

void ObjectSpawner::AddObject(int idx, XMFLOAT3 off, bool isReverse) {
	auto t = new WayPointFollower();
	t->SetChild(baseObjs[idx]);
	t->prevPos = t->pos = isReverse ? 1.f : 0.f;
	t->direc = isReverse ? -1.f : 1.f;
	t->Scale(2, 2, 2);
	t->objectSpawner = this;
	t->offset = RandomPositionInSphere(XMFLOAT3(0, 0, 0), 30, 5, 5);
	if(idx == 0) {
		// 작은 헬기
		t->speed = Random(0.009f, 0.012f);
	}else {
		t->attackRange = Random(0.1f, 0.12f);
		t->durability = 8.0f;
	}
	targetShader->m_ppObjects.push_back(t);

	if(!isReverse)
		Objs0.push_back(t);
	else
		Objs1.push_back(t);
}

void ObjectSpawner::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent) {
	remainSpawnTime0 -= fTimeElapsed;
	remainSpawnTime1 -= fTimeElapsed;
	if(remainSpawnTime0 <= 0) {
		AddObject((int)Random(0, 1.99f), XMFLOAT3(0,0,0));
		remainSpawnTime0 = Random(3, 6);
	}
	if (remainSpawnTime1 <= 0) {
		AddObject((int)Random(0, 1.99f), XMFLOAT3(0, 0, 0), true);
		remainSpawnTime1 = Random(3, 6);
	}

	// Check attack
	
}

////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject() : CGameObject(1) {
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}
void CRotatingObject::Animate(float fTimeElapsed, XMFLOAT4X4 *pxmf4x4Parent) {
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CTerrainWater::CTerrainWater(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, float fWidth, float fLength) : CGameObject(1) {
	CTexturedRectMesh* pWaterMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, fWidth, fLength, 0, 0.0f, 0.0f, 0.0f, 6,6,1);
	pWaterMesh->m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	SetMesh(pWaterMesh);
	Rotate(-90, 0, 0);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture* pWaterTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0, 1);

	pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water_Base_Texture_0.dds", RESOURCE_TEXTURE2D, 0);
	pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water_Detail_Texture_0.dds", RESOURCE_TEXTURE2D, 1);
	pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Lava(Diffuse).dds", RESOURCE_TEXTURE2D, 2);
	//	pWaterTexture->LoadTextureFromDDSFile(pd3dDevice, pd3dCommandList, L"Image/Water_Texture_Alpha.dds", RESOURCE_TEXTURE2D, 2);

	//UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CTerrainWaterShader* pWaterShader = new CTerrainWaterShader();
	pWaterShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	pWaterShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pWaterShader->CreateCbvSrvDescriptorHeaps(pd3dDevice, 1, 3);
	//pWaterShader->CreateConstantBufferViews(pd3dDevice, 1, m_pd3dcbGameObject, ncbElementBytes);
	pWaterShader->CreateShaderResourceViews(pd3dDevice, pWaterTexture, 0, 12);

	CMaterial* pWaterMaterial = new CMaterial();
	pWaterMaterial->SetTexture(pWaterTexture);

	SetMaterial(0, pWaterMaterial);

	//SetCbvGPUDescriptorHandle(pWaterShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pWaterShader);
}

CTerrainWater::~CTerrainWater() {
}
#endif