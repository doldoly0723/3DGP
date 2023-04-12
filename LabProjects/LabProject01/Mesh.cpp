#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;	// 폴리곤(면)을 구성하는 정점의 개수
	m_pVertices = new CVertex[nVertices];	// 정점의 개수만큼 공간 확보
}

CPolygon::~CPolygon()
{
	if (m_pVertices)
		delete[] m_pVertices;
}

// 다각정의 정점을 설정한다
void CPolygon::SetVertex(int nIndex, CVertex vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
		m_pVertices[nIndex] = vertex;
}


CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon* [nPolygons];
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

// 메쉬의 다각형을 설정한다
void CMesh::setPolygon(int nIndex, CPolygon* pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons))
		m_ppPolygons[nIndex] = pPolygon;
}

void Draw2DLine(HDC hDCFrameBuffer, CPoint3D& f3PreviousProject, CPoint3D& f3CurrentProject)
{
	// 투영 좌표계의 2점을 화면 좌표계로 변환하고 변환된 두 점(픽셀)을 선분으로 그린다
	CPoint3D f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	CPoint3D f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	CPoint3D f3InitialProject, f3PreviousProject, f3Intersect;
	bool bPreviousInside = false, bInitialnside = false, bCurrentInside = false, bIntersectInside = false;

	// 메쉬를 구성하는 모든 다각형들을 렌더링한다.
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;
		
		// 다각형의 첫번째 정점을 원근 투영 변환한다
		f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_f3Position);
		// 변환된 점이 투영 사각형에 포함되는 가를 계산한다.
		bPreviousInside = bInitialnside = (-1.0f <= f3InitialProject.x) && (f3InitialProject.x <= 1.0f) &&
			(-1.0f <= f3InitialProject.y) && (f3InitialProject.y <= 1.0f);

		// 다각형을 구성하는 모든 정점들을 원근 투영 변환하고 선분으로 렌더링한다
		for (int i = 1; i < nVertices; i++)
		{
			CPoint3D f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_f3Position);
			// 변환된 점이 투영 사각형에 포함되는 가를 계산한다.
			bCurrentInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) &&
				(-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);
			// 변환된 점이 투영 사각형에 포함되면 이전 점과 현재 점을 선분으로 그린다.
			if (((f3PreviousProject.z >= 0.0f) || (f3CurrentProject.z >= 0.0f)) && ((bCurrentInside || bPreviousInside)))
				::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
			f3PreviousProject = f3CurrentProject;
			bPreviousInside = bCurrentInside;
		}
		// 다각형의 마지막 정점과 다각형의 시작접을 선분으로 그린다.
		if (((f3PreviousProject.z >= 0.0f) || (f3InitialProject.z >= 0.0f)) && ((bInitialnside || bPreviousInside)))
			::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
	}
}
