#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"


CPolygon::CPolygon(int nVertices){
	m_nVertices = nVertices;
	OutputDebugStringW(L"CPolygonstart");
	m_pVertices = new CVertex[nVertices];
	OutputDebugStringW(L"CPolygonEnd");
}
CPolygon::~CPolygon(){
	if(m_pVertices) delete[] m_pVertices;
}
void CPolygon::SetVertex(int nIndex, CVertex vertex){
	if((0<=nIndex)&&(nIndex<m_nVertices)&&m_pVertices){
		m_pVertices[nIndex] = vertex;
	}
}

CMesh::CMesh(int nPolygons){
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon*[nPolygons];
}

CMesh::~CMesh(){
	if(m_ppPolygons){
		for(int i = 0; i<m_nPolygons; i++)
			if(m_ppPolygons[i])
				delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

void CMesh::SetPolygon(int nIndex, CPolygon *pPolygon){
	//메쉬의 다각형을 설정한다.
	if((0<=nIndex)&&(nIndex<m_nPolygons))
		m_ppPolygons[nIndex] = pPolygon;
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject){
	XMFLOAT3 f3Previous =
		CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	XMFLOAT3 f3Current =
		CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y,
		NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

bool GetClipedLine(bool prevInside, bool curInside, XMFLOAT3 &prevPos, XMFLOAT3 &curPos){
	auto prev = XMLoadFloat3(&prevPos);
	auto cur = XMLoadFloat3(&curPos);
	if(XMVector3Equal(prev, cur)) return false;
	float dist;
	XMFLOAT3 center(0, 0, 0.5f);
	XMFLOAT3 size(100, 100, 0.5f);

	if(prevInside||curInside){
		if(prevInside!=curInside){
			if(prevInside){ // 전께 안으로
				auto direc = XMVector3Normalize(prev-cur);
				if(BoundingBox(center, size).Intersects(cur, direc, dist)){
					if(dist>0){
						auto point = cur+direc*dist;
						XMStoreFloat3(&curPos, point);
					}
				}
			} else if(curInside){ // 지금께 안으로
				auto direc = XMVector3Normalize(cur-prev);
				if(BoundingBox(center, size).Intersects(prev, direc, dist)){
					if(dist>0){
						auto point = prev+direc*dist;
						XMStoreFloat3(&prevPos, point);
					}
				}
			}
		}
		return true;
	}

	auto direc = XMVector3Normalize(prev-cur);
	if(BoundingBox(center, size).Intersects(cur, direc, dist)){
		if(dist>0){
			auto point = cur+direc*dist;
			XMStoreFloat3(&curPos, point);
		}
	} else return false;
	direc = XMVector3Normalize(cur-prev);
	if(BoundingBox(center, size).Intersects(prev, direc, dist)){
		if(dist>0){
			auto point = prev+direc*dist;
			XMStoreFloat3(&prevPos, point);
		}
	} else return false;
	return true;

	return false;
}

bool intersectPlane(const XMVECTOR &n, const XMVECTOR &p0, const XMVECTOR &l0, const XMVECTOR &l, float &t){
	// assuming vectors are all normalized
	float denom;
	XMStoreFloat(&denom, XMVector3Dot(n, l));

	if(denom>1e-6){
		auto p0l0 = p0-l0;
		XMStoreFloat(&t, XMVector3Dot(p0l0, n));
		t = t/denom;
		return (t>=0);
	}
	return false;
}

// z:0 보다 더 뒤로 가는것들 교점구함
void GetIntersectPoint(XMFLOAT3& start, XMFLOAT3& end){
	if(start.z>0&&end.z>0) return;
	auto n = XMFLOAT3(0, 0, -1);
	auto p0 = XMFLOAT3(0, 0, 0);
	XMVECTOR l = XMVector3Normalize(XMLoadFloat3(&end)-XMLoadFloat3(&start));
	XMVECTOR l0 = XMLoadFloat3(&start);
	XMFLOAT3* changePoint = &end;
	if(start.z<0){ // end가 화면안 start가 안쪽으로 넘어갈때
		l = -l;
		l0 = XMLoadFloat3(&end);
		changePoint = &start;
	}

	float t;
	auto result = intersectPlane(XMLoadFloat3(&n), XMLoadFloat3(&p0), l0, l, t);
	if(result)
		XMStoreFloat3(changePoint, l0+l*t);
}


void CMesh::Render(HDC hDCFrameBuffer){
	XMFLOAT3 f3InitialProject, f3PreviousProject, f3InitialView, f3PreviousView;
	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;
	for(int j = 0; j<m_nPolygons; j++){
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;
		f3PreviousView = f3InitialView = CGraphicsPipeline::World2View(pVertices[0].m_xmf3Position);
		bPreviousInside = bInitialInside = 0.0f<=f3InitialView.z;
		for(int i = 1; i<nVertices; i++){
			XMFLOAT3 f3CurrentView = CGraphicsPipeline::World2View(pVertices[i].m_xmf3Position);

			XMFLOAT3 t0 = f3PreviousView, t1 = f3CurrentView;
			GetIntersectPoint(t0, t1);
			t0 = CGraphicsPipeline::View2Project(t0);
			t1 = CGraphicsPipeline::View2Project(t1);
			::Draw2DLine(hDCFrameBuffer, t0, t1);

			f3PreviousView = f3CurrentView;
			bPreviousInside = bCurrentInside;
		}
		if(nVertices>2){
			XMFLOAT3 t0 = f3PreviousView, t1 = f3InitialView;
			GetIntersectPoint(t0, t1);
			t0 = CGraphicsPipeline::View2Project(t0);
			t1 = CGraphicsPipeline::View2Project(t1);
			::Draw2DLine(hDCFrameBuffer, t0, t1);		}

		/*f3PreviousProject = f3InitialProject = CGraphicsPipeline::View2Project(f3InitialView);
		bPreviousInside = bInitialInside =
			(-1.0f<=f3InitialProject.x)&&(f3InitialProject.x<=1.0f)&&
			(-1.0f<=f3InitialProject.y)&&(f3InitialProject.y<=1.0f)&&
			(0.0f<=f3InitialProject.z&&f3InitialProject.z<=1.0f);
		for(int i = 1; i<nVertices; i++){
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::World2View(pVertices[i].m_xmf3Position);
			bCurrentInside =
				(-1.0f<=f3CurrentProject.x)&&(f3CurrentProject.x<=1.0f)&&
				(-1.0f<=f3CurrentProject.y)&&(f3CurrentProject.y<=1.0f)&&
				(0.0f<=f3CurrentProject.z&&f3CurrentProject.z<=1.0f);

			XMFLOAT3 t0 = f3PreviousProject, t1 = f3CurrentProject;
			if(GetClipedLine(bPreviousInside, bCurrentInside, t0, t1))
				::Draw2DLine(hDCFrameBuffer, t0, t1);

			f3PreviousProject = f3CurrentProject;
			bPreviousInside = bCurrentInside;
		}
		if(nVertices>2){
			XMFLOAT3 t0 = f3PreviousProject, t1 = f3InitialProject;
			if(GetClipedLine(bPreviousInside, bInitialInside, t0, t1))
				::Draw2DLine(hDCFrameBuffer, t0, t1);		}*/
	}
}

//void CMesh::Render(HDC hDCFrameBuffer){
//	XMFLOAT3 f3InitialProject, f3PreviousProject;
//	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;
//	for(int j = 0; j<m_nPolygons; j++){
//		int nVertices = m_ppPolygons[j]->m_nVertices;
//		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;
//		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
//		bPreviousInside = bInitialInside =
//			(-1.0f<=f3InitialProject.x)&&(f3InitialProject.x<=1.0f)&&
//			(-1.0f<=f3InitialProject.y)&&(f3InitialProject.y<=1.0f);
//		for(int i = 1; i<nVertices; i++){
//			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
//			bCurrentInside =
//				(-1.0f<=f3CurrentProject.x)&&(f3CurrentProject.x<=1.0f)&&
//				(-1.0f<=f3CurrentProject.y)&&(f3CurrentProject.y<=1.0f);
//
//			if(((0.0f<=f3InitialProject.z&&f3InitialProject.z<=1.0f)||(0.0f<=f3CurrentProject.z&&f3CurrentProject.z<=1.0f))&&
//				(bPreviousInside||bCurrentInside)){
//				XMFLOAT3 t0 = f3PreviousProject, t1 = f3CurrentProject;
//				if(!(bPreviousInside&&bCurrentInside))
//					GetIntersectPoint(t0, t1);
//				::Draw2DLine(hDCFrameBuffer, t0, t1);
//			}
//
//			f3PreviousProject = f3CurrentProject;
//			bPreviousInside = bCurrentInside;
//		}
//		if(((0.0f<=f3InitialProject.z&&f3InitialProject.z<=1.0f)||(0.0f<=f3PreviousProject.z&&f3PreviousProject.z<=1.0f))&&
//			((bInitialInside||bPreviousInside))&& nVertices>2){
//			XMFLOAT3 t0 = f3PreviousProject, t1 = f3InitialProject;
//			if(!(bInitialInside&&bPreviousInside))
//				GetIntersectPoint(t0, t1);
//			::Draw2DLine(hDCFrameBuffer, t0, t1);//		}
//	}
//}

//void CMesh::Render(HDC hDCFrameBuffer){
//	XMFLOAT3 f3InitialProject, f3PreviousProject;
//	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;
//	for(int j = 0; j<m_nPolygons; j++){
//		int nVertices = m_ppPolygons[j]->m_nVertices;
//		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;
//		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
//		bPreviousInside = bInitialInside =
//			(-1.0f<=f3InitialProject.x)&&(f3InitialProject.x<=1.0f)&&
//			(-1.0f<=f3InitialProject.y)&&(f3InitialProject.y<=1.0f)&&
//			(0.0f<=f3InitialProject.z&&f3InitialProject.z<=1.0f);
//		for(int i = 1; i<nVertices; i++){
//			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
//			bCurrentInside =
//				(-1.0f<=f3CurrentProject.x)&&(f3CurrentProject.x<=1.0f)&&
//				(-1.0f<=f3CurrentProject.y)&&(f3CurrentProject.y<=1.0f)&&
//				(0.0f<=f3CurrentProject.z&&f3CurrentProject.z<=1.0f);
//
//			XMFLOAT3 t0 = f3PreviousProject, t1 = f3CurrentProject;
//			if(GetClipedLine(bPreviousInside, bCurrentInside, t0, t1))
//				::Draw2DLine(hDCFrameBuffer, t0, t1);
//
//			f3PreviousProject = f3CurrentProject;
//			bPreviousInside = bCurrentInside;
//		}
//		if(nVertices>2){
//			XMFLOAT3 t0 = f3PreviousProject, t1 = f3InitialProject;
//			if(GetClipedLine(bPreviousInside, bInitialInside, t0, t1))
//				::Draw2DLine(hDCFrameBuffer, t0, t1);//		}
//	}
//}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth)
	:CMesh(6){
	int vertexIndex[24] = {0, 1, 2, 3, 0, 4, 5, 1, 1, 5, 6, 2, 0, 3, 7, 4, 5, 4, 7, 6, 2, 6, 7, 3};
	CVertex verts[8] = {{-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, -1, -1}, {-1, -1, -1}};

	int idx = 0;
	for(int i = 0; i<6; i++){
		auto t = new CPolygon(4);
		if(_heapchk()!=_HEAPOK) //힙이 손상 됨을 감지고 하고 해당 위치로 이동
			DebugBreak();
		for(int j = 0; j<4; j++){
			t->m_pVertices[j] = verts[vertexIndex[idx]];
			++idx;
		}

		SetPolygon(i, t);
	}

}

CCubeMesh::~CCubeMesh(){
}

CMapMesh::CMapMesh(): CMesh(40+4*8){
	CVertex verts[4] = {{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0}};

	int totalZ = 50;
	int lineCount = 40;
	float offsetZ = totalZ/(float)lineCount*2;
	float curZ = -totalZ;
	for(int i = 0; i<lineCount; i++){
		auto t = new CPolygon(4);
		for(size_t j = 0; j<4; j++){
			auto& pos = verts[j].m_xmf3Position;
			pos.z = curZ;
			t->SetVertex(j, verts[j]);
		}
		curZ += offsetZ;
		SetPolygon(i, t);
	}
	for(size_t i = 0; i<4; i++){
		verts[i].m_xmf3Position.z = 0;
	}
	for(size_t i = 0; i<4; i++){ // 방향결정
		auto direc = (XMLoadFloat3(&verts[(i+1)%4].m_xmf3Position)-XMLoadFloat3(&verts[i].m_xmf3Position)) / 8.0f;
		auto startPos = XMLoadFloat3(&verts[i].m_xmf3Position);
		for(size_t j = 0; j<8; j++){ // 방향에 맞춰만들기
			auto t = new CPolygon(2);
			XMFLOAT3 tem;
			CVertex temp;
			startPos = XMVectorSetZ(startPos, -totalZ);
			XMStoreFloat3(&tem, startPos);
			temp.m_xmf3Position = tem;
			t->SetVertex(0, temp);
			startPos = XMVectorSetZ(startPos, totalZ);
			XMStoreFloat3(&tem, startPos);
			temp.m_xmf3Position = tem;
			t->SetVertex(1, temp);
			SetPolygon(i*8+j+lineCount, t);
			startPos += direc;
		}
		
	}
}

CMapMesh::~CMapMesh(){
}

CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth)
	: CMesh(24){
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	float x1 = fx*0.2f, y1 = fy*0.2f, x2 = fx*0.1f, y3 = fy*0.3f,
		y2 = ((y1-(fy-y3))/x1) * x2+(fy-y3);
	int i = 0;
	//비행기 메쉬의 위쪽 면
	CPolygon *pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);
	//비행기 메쉬의 아래쪽 면
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);

	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);
	//비행기 메쉬의 오른쪽 면
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);
	//비행기 메쉬의 뒤쪽/오른쪽 면
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);

	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);
	//비행기 메쉬의 왼쪽 면
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy+y3), -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy+y3), +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);
	//비행기 메쉬의 뒤쪽/왼쪽 면
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);
	pFace = new CPolygon(3);

	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);
}

CLineMesh::CLineMesh() : CMesh(1){
	CVertex verts[2] = {{0, 0, 10}, {0, 0, -10}};
	auto t = new CPolygon(2);
	t->SetVertex(0, verts[0]);
	t->SetVertex(1, verts[1]);
	SetPolygon(0, t);
}
