#pragma once


class CPoint3D
{
public:
	CPoint3D() { }
	CPoint3D(float x, float y, float z) {
		this->pos.x = x; this->pos.y = y;
		this->pos.z = z;
	}
	virtual ~CPoint3D() { }
	XMFLOAT3 pos;
};
class CVertex
{
public:
	CVertex() { }
	CVertex(float x, float y, float z) {
		m_f3Position = CPoint3D(x, y, z);
	}
	virtual ~CVertex() { }
	CPoint3D m_f3Position;
};
class CPolygon
{
public:
	CPolygon() { }
	CPolygon(int nVertices);
	virtual ~CPolygon();

	//�ٰ���(��)�� �����ϴ� �������� ����Ʈ�̴�.
	int m_nVertices = 0;
	CVertex *m_pVertices = NULL;

	void SetVertex(int nIndex, CVertex vertex);
};


class CMesh
{
public:
	CMesh() { }
	CMesh(int nPolygons);
	virtual ~CMesh();
private:
	//�ν��Ͻ�(Instancing)�� ���Ͽ� �޽��� ���� ��ü�鿡 ������ �� �ִ�.
	//���� ������(Reference Count)�� �޽��� �����Ǵ� ���� ��ü�� ������ ��Ÿ����.
	int m_nReferences = 1;
public:
	//�޽��� ���� ��ü�� ������ ������ �������� 1�� ������Ų��.
	void AddRef() { m_nReferences++; }
	//�޽��� �����ϴ� ���� ��ü�� �Ҹ�� ������ �������� 1�� ���ҽ�Ų��.
	//�������� 0�̵Ǹ� �޽��� �Ҹ��Ų��.
	void Release() {
		m_nReferences--;
		if (m_nReferences <= 0)
			delete this;
	}
private:
	//�޽��� �����ϴ� �ٰ���(��)���� ����Ʈ�̴�.
	int m_nPolygons = 0;
	CPolygon **m_ppPolygons = NULL;
public:
	void SetPolygon(int nIndex, CPolygon *pPolygon);
	//�޽��� �������Ѵ�.
	virtual void Render(HDC hDCFrameBuffer);
};
//������ü Ŭ������ �����Ѵ�.
class CCubeMesh : public CMesh
{
public:
	CCubeMesh(float fWidth = 4.0f, float fHeight = 4.0f, float fDepth
		= 4.0f);
	virtual ~CCubeMesh();
};

class CMapMesh : public CMesh {
public:
	CMapMesh();
	virtual ~CMapMesh();
};

class CAirplaneMesh : public CMesh {
public:
	CAirplaneMesh(float fWidth, float fHeight, float fDepth);
	virtual ~CAirplaneMesh() {}
};