#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

void CScene::BuildObjects() {
	CCubeMesh* pCubeMesh = &CCubeMesh::getInstance();
	m_pObjects.resize(6);
	m_pObjects[0] = new CExplosedObjects();
	m_pObjects[0]->SetMesh(pCubeMesh);
	m_pObjects[0]->SetColor(RGB(255, 0, 0));
	m_pObjects[0]->SetPosition(-13.5f, 0.0f, +14.0f);
	m_pObjects[0]->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 0.0f));
	m_pObjects[0]->SetRotationSpeed(90.0f);
	m_pObjects[0]->SetMovingDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	m_pObjects[0]->SetMovingSpeed(0.5f);
	m_pObjects[1] = new CExplosedObjects();
	m_pObjects[1]->SetMesh(pCubeMesh);
	m_pObjects[1]->SetColor(RGB(0, 0, 255));
	m_pObjects[1]->SetPosition(+13.5f, 0.0f, +14.0f);
	m_pObjects[1]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_pObjects[1]->SetRotationSpeed(180.0f);
	m_pObjects[1]->SetMovingDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	m_pObjects[1]->SetMovingSpeed(1.5f);
	m_pObjects[2] = new CExplosedObjects();
	m_pObjects[2]->SetMesh(pCubeMesh);
	m_pObjects[2]->SetColor(RGB(0, 255, 0));
	m_pObjects[2]->SetPosition(0.0f, +5.0f, 20.0f);
	m_pObjects[2]->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 1.0f));
	m_pObjects[2]->SetRotationSpeed(30.15f);
	m_pObjects[2]->SetMovingDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));
	m_pObjects[2]->SetMovingSpeed(0.0f);
	m_pObjects[3] = new CExplosedObjects();
	m_pObjects[3]->SetMesh(pCubeMesh);
	m_pObjects[3]->SetColor(RGB(0, 255, 255));
	m_pObjects[3]->SetPosition(0.0f, 0.0f, 40.0f);
	m_pObjects[3]->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_pObjects[3]->SetRotationSpeed(40.6f);
	m_pObjects[3]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	m_pObjects[3]->SetMovingSpeed(0.0f);
	m_pObjects[4] = new CExplosedObjects();
	m_pObjects[4]->SetMesh(pCubeMesh);
	m_pObjects[4]->SetColor(RGB(128, 0, 255));
	m_pObjects[4]->SetPosition(10.0f, 10.0f, 50.0f);
	m_pObjects[4]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_pObjects[4]->SetRotationSpeed(50.06f);
	m_pObjects[4]->SetMovingDirection(XMFLOAT3(0.0f, 1.0f, 1.0f));
	m_pObjects[4]->SetMovingSpeed(0.0f);
	m_pObjects[5] = new CGameObject();
	m_pObjects[5]->SetMesh(new CMapMesh());
	m_pObjects[5]->radius = -1000;
}
void CScene::ReleaseObjects() {
	for (size_t i = 0; i < m_pObjects.size(); i++) if (m_pObjects[i]) delete m_pObjects[i];
	m_pObjects.clear();
}

void CScene::Animate(float fElapsedTime) {
	for (size_t i = 0; i < m_pObjects.size(); i++)
		m_pObjects[i]->Animate(fElapsedTime);
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera) {
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewProjectTransform(&pCamera->m_xmf4x4ViewProject);
	for (size_t i = 0; i < m_pObjects.size(); i++)
		m_pObjects[i]->Render(hDCFrameBuffer, pCamera);
}

CGameObject* CScene::CheckCollision(const CGameObject* rhs){
	for(auto& t:m_pObjects){
		auto obj = t->CheckCollision(rhs);
		if(obj!=nullptr) return obj;
	}
	return nullptr;
}

CGameObject * CScene::CheckCollision(XMFLOAT3 origin, XMFLOAT3 direc){
	for(auto& t:m_pObjects){
		auto obj = t->CheckCollision(origin, direc);
		if(obj!=nullptr) return obj;
	}
	return nullptr;
}
