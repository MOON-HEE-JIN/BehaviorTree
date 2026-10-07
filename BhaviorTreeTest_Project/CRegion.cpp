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

	m_Rect = rect;
	m_iRenderSize = RenderSize;
	m_iHalfRenderSize = RenderSize / 2;
	m_hRenderPen = (HPEN)CreatePen(BS_SOLID, 1, RGB(0, 255, 0));
	m_hRenderRedPen = (HPEN)CreatePen(BS_SOLID, 1, RGB(255, 0, 0));
	m_hRenderBluePen = (HPEN)CreatePen(BS_SOLID, 1, RGB(0, 0, 255));
	m_hRenderBrushBG = CreateSolidBrush(RGB(37, 37, 38));
	m_hRenderBrushCloseDoor = CreateSolidBrush(RGB(0, 255, 0));
	m_iIndex = index;

	m_iOccupyTeam = eTeam::NONE;
	m_iCurOccupyingTeam = eTeam::NONE;
	m_iOccupyTeamCount = 0;
	m_iOppositeCount = 0;
	m_iOccupyTimer = rect.Width * rect.Height * 1000; // 넓이 * 1000
	m_iOccupyingTime = 0;
}

void CRegion::AddParts(CPartsBase* pPart)
{
	m_vecRenderParts.push_back(pPart);
	m_mapParts[pPart->GetType()].push_back(pPart);
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
	switch (m_iOccupyTeam)
	{
	case eTeam::RED:
		brush.SetColor(Color(128, 255, 0, 0));
		SelectObject(hdc, m_hRenderRedPen);
		break;
	case eTeam::BLUE:
		brush.SetColor(Color(128, 0, 0, 255));
		SelectObject(hdc, m_hRenderBluePen);
		break;
	}
	
	Rectangle(hdc, m_Rect.RLeft, m_Rect.RTop, m_Rect.RRight, m_Rect.RBottom);
	
	if (m_iCurOccupyingTeam != eTeam::NONE)
	{
		switch (m_iCurOccupyingTeam)
		{
		case eTeam::RED:
			brush.SetColor(Color(128, 255, 0, 0));
			break;
		case eTeam::BLUE:
			brush.SetColor(Color(128, 0, 0, 255));
			break;
		}
		Gdiplus::Rect rect(m_Rect.RLeft, m_Rect.RTop, m_Rect.Width * m_iRenderSize * m_iProgress, m_Rect.Height * m_iRenderSize);

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
	return st_Pos(Random(m_Rect.Left + 1, m_Rect.Right - 1), Random(m_Rect.Top + 1, m_Rect.Bottom - 1));
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
	if (m_iOccupyTeam == eTeam::NONE)
	{
		if (m_iCurOccupyingTeam == eTeam::NONE)
			EnterNoneRegion(team);
		else
			EnterOccupyingRegion(team);
	}
	else
		EnterOccupyRegion(team);

	printf("Enter Region[%d] eTeam %s\n", m_iIndex, team == eTeam::RED ? "Red" : "Blue");
}

void CRegion::ExitRegion(eTeam team, CEntity* pObj)
{
	if (m_iCurOccupyingTeam == team)
		m_iOccupyTeamCount--;
	else
		m_iOppositeCount--;
}

void CRegion::OccupyRegion()
{
	m_iOccupyTeam = m_iCurOccupyingTeam;
	printf("Occupy Region[%d] eTeam %s\n", m_iIndex, m_iOccupyTeam == eTeam::RED ? "Red" : "Blue"); 
}

void CRegion::UpdateOccupyTime()
{
	if (m_iOccupyTeam == m_iCurOccupyingTeam)
		return;

	if (m_iCurOccupyingTeam == eTeam::NONE)
		return;
	
	m_iOccupyingTime += 1000;

	m_iProgress = (float)m_iOccupyingTime / m_iOccupyTimer;

	if (m_iProgress > 1.0f)
		m_iProgress = 1.0f;

	if (m_iOccupyTimer <= m_iOccupyingTime)
		OccupyRegion();
}

void CRegion::UpdateInRegion()
{}


void CRegion::DebugFunc_Test(eTeam occupyTeam, eTeam curoccupyingteam, int occupyTeamCount, int oppositeCount, int occupyTimer, int occupyingTime)

{
	m_iOccupyTeam = occupyTeam;
	m_iCurOccupyingTeam = curoccupyingteam;
	m_iOccupyTeamCount = occupyTeamCount;
	m_iOppositeCount = oppositeCount;
	m_iOccupyingTime = occupyingTime;
}

void CRegion::EnterNoneRegion(eTeam team)
{
	m_iCurOccupyingTeam = team;
	m_iOccupyTeamCount++;
}

void CRegion::EnterOccupyingRegion(eTeam team)
{
	if (team == m_iCurOccupyingTeam)
		m_iOccupyTeamCount++;
	else
		m_iOppositeCount++;
}

void CRegion::EnterOccupyRegion(eTeam team)
{
	if (team == m_iOccupyTeam)
		m_iOppositeCount++;
	else
	{
		m_iCurOccupyingTeam = team;
		m_iOccupyTeamCount++;
	}
}

void CRegion::ChangeOccupyTeam()
{
	m_iCurOccupyingTeam = m_iCurOccupyingTeam == eTeam::RED ? eTeam::BLUE : eTeam::RED;

	m_iOccupyTeamCount = m_iOppositeCount;
	m_iOppositeCount = 0;
	m_iOccupyingTime = 0;
}
