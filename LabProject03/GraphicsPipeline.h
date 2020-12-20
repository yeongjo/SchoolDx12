#pragma once

#include "GameObject.h"
#include "Camera.h"

class CGraphicsPipeline {
private:
	//현재 렌더링할 게임 객체의 월드 변환 행렬이다. 
	static XMFLOAT4X4* m_pxmf4x4World;
	//현재 카메라 객체의 (카메라 변환 행렬 * 원근 투영 변환 행렬)이다. 
	static XMFLOAT4X4* m_pxmf4x4ViewProject;

	static BoundingFrustum boundingFrustum;
public:

	static XMFLOAT4X4* m_pxmf4x4View;
	static XMFLOAT4X4* m_pxmf4x4Project;

	static void SetWorldTransform(XMFLOAT4X4* pxmf4x4World) {
		m_pxmf4x4World = pxmf4x4World;
	}
	static void SetViewProjectTransform(XMFLOAT4X4* pxmf4x4ViewProject) {
		m_pxmf4x4ViewProject = pxmf4x4ViewProject;
		boundingFrustum = BoundingFrustum(XMLoadFloat4x4(m_pxmf4x4ViewProject));
	}
	static XMFLOAT3 ScreenTransform(XMFLOAT3& xmf3Project);
	static XMFLOAT3 Viewport2World(XMFLOAT3& viewport);
	static XMFLOAT3 Project(XMFLOAT3& xmf3Model);
	static XMFLOAT3 World2View(XMFLOAT3& xmf3Model);
	static XMFLOAT3 View2Project(XMFLOAT3& xmf3Model);
	// 화면좌표에서 -1~1좌표로 이동
	static XMFLOAT3 Screen2ViewPort(XMFLOAT3& screen);
	static BoundingFrustum GetBoundingFrustum(){
		return boundingFrustum;
	}
};

