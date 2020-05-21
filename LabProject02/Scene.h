#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "Player.h"

class CScene {
public:
	CScene(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	virtual ~CScene() {}
private:
	vector<CGameObject*> m_pObjects;
	CPlayer* m_pPlayer = NULL;
public:
	virtual void BuildObjects();
	virtual void ReleaseObjects();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
	//윈도우 메시지(키보드, 마우스)를 처리한다. 
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
	CGameObject* CheckCollision(const CGameObject* rhs);
	CGameObject* CheckCollision(XMFLOAT3 origin, XMFLOAT3 direc);
};