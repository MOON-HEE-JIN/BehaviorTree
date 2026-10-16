#include "CRegion.h"
#include "CEntity.h"

#include <Windows.h>
#include <gdiplus.h>
#include <string>

#include "GUtill.h"
#include "WinThread.h"


#include "CDoor.h"

using namespace Gdiplus;

CRegion::CRegion(st_Rect rect, int RenderSize, int index)
{
	SetUpdateTime(100);

	m_stPos = rect.Center;

	m_rect = rect;
	m_iRenderSize = RenderSize;
	m_hRenderPen = (HPEN)CreatePen(BS_SOLID, 1, RGB(0, 255, 0));
	m_hRenderRedPen = (HPEN)CreatePen(BS_SOLID, 1, RGB(255, 0, 0));
	m_hRenderBluePen = (HPEN)CreatePen(BS_SOLID, 1, RGB(0, 0, 255));
	m_iIndex = index;

	m_eOccupyTeam = eTeam::None;
	m_eCurOccupyingTeam = eTeam::None;
	m_iOccupyTeamCount = 0;
	m_iOppositeCount = 0;
	m_iOccupyTimer = rect.Width * rect.Height * 1000; // 넓이 * 1000
	m_iOccupyingTime = 0;
}

void CRegion::AddParts(CPartsBase* pPart)
{
	m_vecRenderParts.push_back(pPart);
}

void CRegion::Update()
{
	UpdateOccupyTime();
	UpdateInRegion();
}

void CRegion::Render(HDC hdc)
{
	Graphics g(hdc);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	SelectObject(hdc, m_hRenderPen);

	SolidBrush brush(Color(0, 0, 0, 0));
	switch (m_eOccupyTeam)
	{
	case eTeam::Red:
		brush.SetColor(Color(128, 255, 0, 0));
		SelectObject(hdc, m_hRenderRedPen);
		break;
	case eTeam::Blue:
		brush.SetColor(Color(128, 0, 0, 255));
		SelectObject(hdc, m_hRenderBluePen);
		break;
	}

	Rectangle(hdc, m_rect.RLeft, m_rect.RTop, m_rect.RRight, m_rect.RBottom);

	if (m_eCurOccupyingTeam != eTeam::None)
	{
		switch (m_eCurOccupyingTeam)
		{
		case eTeam::Red:
			brush.SetColor(Color(128, 255, 0, 0));
			break;
		case eTeam::Blue:
			brush.SetColor(Color(128, 0, 0, 255));
			break;
		}
		Gdiplus::Rect rect(m_rect.RLeft, m_rect.RTop, m_rect.Width * m_iRenderSize * m_fProgress, m_rect.Height * m_iRenderSize);

		g.FillRectangle(&brush, rect);
	}

	int nLoop = m_vecRenderParts.size();
	for (int i = 0; i < nLoop; i++)
	{
		m_vecRenderParts[i]->Render(hdc);
	}
}

st_Pos CRegion::GetRegionInsidePos()
{
	// 사각형 안쪽
	return st_Pos(Random(m_rect.Left + 1, m_rect.Right - 1), Random(m_rect.Top + 1, m_rect.Bottom - 1));
}

st_Pos CRegion::GetRandomRegionDoor()
{
	return st_Pos();
}

st_Pos CRegion::GetRegionDoor(eDirection eDir)
{
	return st_Pos();
}

void CRegion::EnterRegion(eTeam team, CEntity* pObj)
{
	if (m_eOccupyTeam == eTeam::None)
	{
		if (m_eCurOccupyingTeam == eTeam::None)
			EnterNoneRegion(team);
		else
			EnterOccupyingRegion(team);
	}
	else
		EnterOccupyRegion(team);

	m_mapTeamEntity[team].push_back(pObj);
	printf("Enter Region[%d] eTeam %s\n", m_iIndex, team == eTeam::Red ? "Red" : "Blue");
}

void CRegion::ExitRegion(eTeam team, CEntity* pObj)
{
	if (m_eCurOccupyingTeam == team)
		m_iOccupyTeamCount--;
	else
		m_iOppositeCount--;
	
	// 추후 해당 자료구조 수정
	std::vector<CEntity*>::iterator biter = m_mapTeamEntity[team].begin();
	std::vector<CEntity*>::iterator eiter = m_mapTeamEntity[team].end();
	for (biter; biter != eiter; ++biter)
	{
		if ((*biter) == pObj)
		{
			m_mapTeamEntity[team].erase(biter);
			break;
		}
	}
	
}

void CRegion::OccupyRegion()
{
	m_eOccupyTeam = m_eCurOccupyingTeam;
	printf("Occupy Region[%d] eTeam %s\n", m_iIndex, m_eOccupyTeam == eTeam::Red ? "Red" : "Blue");
	m_iOccupyingTime = 0;
}

void CRegion::UpdateOccupyTime()
{
	if (m_eOccupyTeam == m_eCurOccupyingTeam)
		return;

	if (m_eCurOccupyingTeam == eTeam::None)
		return;

    // 점령 규칙 검토: Update 주기 100ms에 비해 호출마다 3000ms 증가한다. 의도한 배속인지 확인이 필요하다.
    // 점령 규칙 검토: 점령 인원이 0이거나 적군이 있어도 진행 시간을 증가시킨다.
	m_iOccupyingTime += 3000;

	m_fProgress = (float)m_iOccupyingTime / m_iOccupyTimer;

	if (m_fProgress > 1.0f)
		m_fProgress = 1.0f;

	if (m_iOccupyTimer <= m_iOccupyingTime)
		OccupyRegion();
}

void CRegion::UpdateInRegion()
{}


void CRegion::DebugFunc_Test(eTeam occupyTeam, eTeam curoccupyingteam, int occupyTeamCount, int oppositeCount, int occupyTimer, int occupyingTime)

{
	m_eOccupyTeam = occupyTeam;
	m_eCurOccupyingTeam = curoccupyingteam;
	m_iOccupyTeamCount = occupyTeamCount;
	m_iOppositeCount = oppositeCount;
	m_iOccupyingTime = occupyingTime;
}

void CRegion::EnterNoneRegion(eTeam team)
{
	m_eCurOccupyingTeam = team;
	m_iOccupyTeamCount++;
}

void CRegion::EnterOccupyingRegion(eTeam team)
{
	if (team == m_eCurOccupyingTeam)
		m_iOccupyTeamCount++;
	else
		m_iOppositeCount++;
}

void CRegion::EnterOccupyRegion(eTeam team)
{
	if (team == m_eOccupyTeam)
		m_iOppositeCount++;
	else
	{
		m_eCurOccupyingTeam = team;
		m_iOccupyTeamCount++;
	}
}
