﻿#include "stdafx.h"
#include "Shader.h"
#include "GameObject.h"

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
}
CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		m_pShader->Release();
	}
}

void CGameObject::SetShader(CShader *pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}
void CGameObject::SetMesh(CMesh *pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::ReleaseUploadBuffers()
{
	//정점 버퍼를 위한 업로드 버퍼를 소멸시킨다. 
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
void CGameObject::Animate(float fTimeElapsed)
{
}
void CGameObject::OnPrepareRender()
{
}
void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	OnPrepareRender();
	//게임 객체에 셰이더 객체가 연결되어 있으면 셰이더 상태 객체를 설정한다. 
	if (m_pShader) m_pShader->Render(pd3dCommandList);
	//게임 객체에 메쉬가 연결되어 있으면 메쉬를 렌더링한다. 
	if (m_pMesh) m_pMesh->Render(pd3dCommandList);
}