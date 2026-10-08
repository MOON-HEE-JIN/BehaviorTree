#include "CEntity.h"
#include <Windows.h>
#include <gdiplus.h>
#include <string>
#include <math.h>
#include <list>
#include "WinThread.h"
using namespace Gdiplus;

CEntity::CEntity(eTeam team, int id)
{
	m_eTeam = team;
	m_iID = id;
	m_iCurrentRegionIndex = -1;

	switch (team)
	{
	case eTeam::NONE:
		break;
	case eTeam::RED:
		m_hRenderBrush = CreateSolidBrush(RGB(100, 20, 20));
		break;
	case eTeam::BLUE:
		m_hRenderBrush = CreateSolidBrush(RGB(20, 20, 100));
		break;
	}

	SetRenderPrivateSize(8);

	SetUpdateTime(500);
}

void CEntity::Init(st_Pos& spawnpos)
{
	SetPos(spawnpos);
	m_eState = eState::SPAWN;

	m_Ai.SetTarget(this);
}

void CEntity::Update()
{
	m_Ai.Update();
}

void CEntity::Render(HDC hdc)
{
	SelectObject(hdc, m_hRenderBrush);
	Ellipse(hdc
		, m_stRenderPos.X - GetRenderHalfSize() - GetRenderPrivateHalfSize()
		, m_stRenderPos.Y - GetRenderHalfSize() - GetRenderPrivateHalfSize()
		, m_stRenderPos.X + GetRenderHalfSize() + GetRenderPrivateHalfSize()
		, m_stRenderPos.Y + GetRenderHalfSize() + GetRenderPrivateHalfSize());
}

void CEntity::OnSpawn()
{
	ChangeState(eState::IDLE);
	m_Ai.SetMap(g_SimulatorManager->GetMap());
	m_Ai.SetTarget(this);

	m_iHP = 100;
}

bool CEntity::GetMoveNextPosInfo(st_PosInfo& posinfo)
{
	if (m_queueMovePath.empty())
	{
		if (!NetMovePath())
			return eBTresult::FAILD;
	}
	posinfo = m_queueMovePath.front();
	
	return eBTresult::SUCCESS;
}

void CEntity::InitAstarPathtoMovePath()
{
	st_Pos nodePos = m_vecAstarPath.back();
	m_vecAstarPath.pop_back();

	m_Ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
}

void CEntity::ChangeState(eState state)
{
	m_Ai.m_board.ResetRuntime();

	m_eState = state;
}

bool CEntity::NetMovePath()
{
	if (m_vecAstarPath.empty() || !m_queueMovePath.empty())
		return eBTresult::FAILD;

	st_Pos nodePos = m_vecAstarPath.back();
	m_vecAstarPath.pop_back();

	m_Ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
	if (m_queueMovePath.empty())
		return eBTresult::FAILD;

	return eBTresult::SUCCESS;
}

int CEntity::Move()
{
	if (m_vecAstarPath.empty() && m_queueMovePath.empty())
		return eBTresult::SUCCESS;

	if (m_queueMovePath.empty())
	{
		st_Pos nodePos = m_vecAstarPath.back();
		m_vecAstarPath.pop_back();

		m_Ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
		if (m_queueMovePath.empty())
			return eBTresult::FAILD;
	}

	st_Pos OldPos = m_stPos;

	SetPos(m_queueMovePath.front().pos);

	int oldRegionIndex = m_Ai.GetMap()->GetRegionIndex(OldPos);
	m_iCurrentRegionIndex = m_Ai.GetMap()->GetRegionIndex(m_stPos);

	if (oldRegionIndex != m_iCurrentRegionIndex)
	{
		CRegion* pOldRegion = m_Ai.GetMap()->GetRegion(oldRegionIndex);
		if (pOldRegion)
			pOldRegion->ExitRegion(m_eTeam, this);
		CRegion* pNewRegion = m_Ai.GetMap()->GetRegion(m_iCurrentRegionIndex);
		if (pNewRegion)
			pNewRegion->EnterRegion(m_eTeam, this);
	
		if (m_iCurrentRegionIndex != -1)
		{
			ChangeState(eState::OCCUPY);
			m_Ai.m_board.m_iOccupyRegionIndex = m_iCurrentRegionIndex;	
		}
	}

	m_queueMovePath.pop();

	return eBTresult::RUNNING;
}
