#include "stdafx.h"
#include "GraphicsPipeline.h"

XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4World = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4ViewProject = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4View = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4Project = NULL;
CViewport* CGraphicsPipeline::m_pViewport = NULL;BoundingFrustum CGraphicsPipeline::boundingFrustum;

XMFLOAT3 CGraphicsPipeline::Project(XMFLOAT3& xmf3Model){
	//XMMATRIX xmmtxModelToProject =
	//	XMMatrixMultiply(XMLoadFloat4x4(m_pxmf4x4World),
	//		XMLoadFloat4x4(m_pxmf4x4ViewProject));
	//XMFLOAT3 xmf3Project;
	//XMStoreFloat3(&xmf3Project,
	//	XMVector3TransformCoord(XMLoadFloat3(&xmf3Model),
	//		xmmtxModelToProject));
	//return(xmf3Project);


	XMMATRIX xmmtxModelToProject =
		XMMatrixMultiply(XMLoadFloat4x4(m_pxmf4x4World),
			XMLoadFloat4x4(m_pxmf4x4ViewProject));
	XMFLOAT4 t(xmf3Model.x, xmf3Model.y, xmf3Model.z, 1);
	XMFLOAT3 xmf3Project;
	XMStoreFloat4(&t,
		XMVector4Transform(XMLoadFloat4(&t), xmmtxModelToProject));

	if(t.z!=0){
		if(t.w>0){
			xmf3Project.x = t.x/t.w;
			xmf3Project.y = t.y/t.w;
			xmf3Project.z = t.z/t.w;
		} else{
			xmf3Project.x = t.x*100.f;
			xmf3Project.y = t.y*100.f;
			xmf3Project.z = t.z/-t.w;
		}
	} else{
		xmf3Project.x = t.x*100.f;
		xmf3Project.y = t.y*100.f;
		xmf3Project.z = 0;
	}

	return(xmf3Project);



}

XMFLOAT3 CGraphicsPipeline::World2View(XMFLOAT3 & xmf3Model){
	XMMATRIX xmmtxModelToView =
		XMMatrixMultiply(XMLoadFloat4x4(m_pxmf4x4World),
			XMLoadFloat4x4(m_pxmf4x4View));
	XMFLOAT4 t(xmf3Model.x, xmf3Model.y, xmf3Model.z, 1);
	XMFLOAT3 xmf3View;
	XMStoreFloat3(&xmf3View,
		XMVector4Transform(XMLoadFloat4(&t), xmmtxModelToView));
	return xmf3View;
}

XMFLOAT3 CGraphicsPipeline::View2Project(XMFLOAT3 & xmf3Model){
	XMMATRIX xmmtxModelToView = XMLoadFloat4x4(m_pxmf4x4Project);
	XMFLOAT4 t(xmf3Model.x, xmf3Model.y, xmf3Model.z, 1);
	XMFLOAT3 xmf3View;
	XMStoreFloat4(&t,
		XMVector4Transform(XMLoadFloat4(&t), xmmtxModelToView));
	if(t.z!=0){
		if(t.w>0){
			xmf3View.x = t.x/t.w;
			xmf3View.y = t.y/t.w;
			xmf3View.z = t.z/t.w;
		} else{
			xmf3View.x = t.x*100.f;
			xmf3View.y = t.y*100.f;
			xmf3View.z = t.z/-t.w;
		}
	} else{
		xmf3View.x = t.x*100.f;
		xmf3View.y = t.y*100.f;
		xmf3View.z = 0;
	}
	return xmf3View;
}

XMFLOAT3 CGraphicsPipeline::Screen2ViewPort(XMFLOAT3 & screen){
	XMFLOAT3 f3Viewport = screen;
	float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;
	f3Viewport.x = (f3Viewport.x-m_pViewport->m_nLeft-fHalfWidth)/fHalfWidth;
	f3Viewport.y = (f3Viewport.y-m_pViewport->m_nTop-fHalfHeight)/-fHalfHeight;
	return f3Viewport;
}

XMFLOAT3 CGraphicsPipeline::ScreenTransform(XMFLOAT3& xmf3Project){
	XMFLOAT3 f3Screen = xmf3Project;
	float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;
	f3Screen.x = m_pViewport->m_nLeft+(xmf3Project.x * fHalfWidth)+
		fHalfWidth;
	f3Screen.y = m_pViewport->m_nTop+(-xmf3Project.y * fHalfHeight)+
		fHalfHeight;
	return(f3Screen);
}

XMFLOAT3 CGraphicsPipeline::Viewport2World(XMFLOAT3 & viewport){
	//XMMATRIX xmmtxViewport2World =
	//	XMMatrixMultiply(XMLoadFloat4x4(m_pxmf4x4World),
	//		XMLoadFloat4x4(m_pxmf4x4ViewProject));
	XMMATRIX xmmtxViewport2World = XMLoadFloat4x4(m_pxmf4x4ViewProject);
	auto invmat = XMMatrixInverse(nullptr, xmmtxViewport2World);
	auto tem = invmat*xmmtxViewport2World;
	XMFLOAT3 xmf3World;
	XMStoreFloat3(&xmf3World,
		XMVector3TransformCoord(XMLoadFloat3(&viewport),
			invmat));
	return(xmf3World);
}
