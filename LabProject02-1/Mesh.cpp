#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"


CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;
	OutputDebugStringW(L"CPolygonstart");
	m_pVertices = new CVertex[nVertices];
	OutputDebugStringW(L"CPolygonEnd");
}
CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}
void CPolygon::SetVertex(int nIndex, CVertex vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}

CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon*[nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons)
	{
		for (int i = 0; i < m_nPolygons; i++)
			if (m_ppPolygons[i])
				delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

void CMesh::SetPolygon(int nIndex, CPolygon *pPolygon)
{
	//�޽��� �ٰ����� �����Ѵ�.
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) 
		m_ppPolygons[nIndex] = pPolygon;
}

void Draw2DLine(HDC hDCFrameBuffer, CPoint3D& f3PreviousProject, CPoint3D& f3CurrentProject)
{
	//���� ��ǥ���� 2���� ȭ�� ��ǥ��� ��ȯ�ϰ� ��ȯ�� �� ��(�ȼ�)�� �������� �׸���.
	CPoint3D f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	CPoint3D f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	CPoint3D f3InitialProject, f3PreviousProject, f3Intersect;
	bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;

	//�޽��� �����ϴ� ��� �ٰ������� �������Ѵ�.
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;
		//�ٰ����� ù ��° ������ ���� ���� ��ȯ�Ѵ�.
		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_f3Position);
		//��ȯ�� ���� ���� �簢���� ���ԵǴ� ���� ����Ѵ�.
		bPreviousInside = bInitialInside =
			(-1.0f <= f3InitialProject.x) && (f3InitialProject.x <= 1.0f) &&
			(-1.0f <= f3InitialProject.y) && (f3InitialProject.y <= 1.0f);

		//�ٰ����� �����ϴ� ��� �������� ���� ���� ��ȯ�ϰ� �������� �������Ѵ�.
		for (int i = 1; i < nVertices; i++)
		{
			CPoint3D f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_f3Position);

			//��ȯ�� ���� ���� �簢���� ���ԵǴ� ���� ����Ѵ�.
			bCurrentInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) &&
				(-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);

			//��ȯ�� ���� ���� �簢���� ���ԵǸ� ���� ���� ���� ���� �������� �׸���
			if (((f3PreviousProject.z >= 0.0f) || (f3CurrentProject.z >= 0.0f)) && ((bCurrentInside || bPreviousInside)))
				::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
			f3PreviousProject = f3CurrentProject;
			bPreviousInside = bCurrentInside;
		}
		//�ٰ����� ������ ������ �ٰ����� �������� �������� �׸���.
		if (((f3PreviousProject.z >= 0.0f) || (f3InitialProject.z >= 0.0f)) && ((bInitialInside || bPreviousInside)))
			::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
	}
}

CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth)
:CMesh(6){
	int vertexIndex[24] = { 0, 1, 2, 3, 0, 4, 5, 1, 1, 5, 6, 2, 0, 3, 7, 4, 5, 4, 7, 6, 2, 6, 7, 3 };
	CVertex verts[8] = { { -1, 1, 1 }, { 1, 1, 1 }, { 1, 1, -1 }, { -1, 1, -1 }, { -1, -1, 1 }, { 1, -1, 1 }, { 1, -1, -1 }, { -1, -1, -1 } };

	int idx = 0;
	for (int i = 0; i < 6; i++)
	{
		OutputDebugStringW(L"start");
		auto t = new CPolygon(4);
		if (_heapchk() != _HEAPOK) //���� �ջ� ���� ������ �ϰ� �ش� ��ġ�� �̵�
			DebugBreak();
		OutputDebugStringW(L"mid");
		for (int j = 0; j < 4; j++)
		{
			t->m_pVertices[j] = verts[vertexIndex[idx]];
			++idx;
		}
		
		SetPolygon(i, t);
		OutputDebugStringW(L"end\n");
	}
	
}

CCubeMesh::~CCubeMesh()
{
}

