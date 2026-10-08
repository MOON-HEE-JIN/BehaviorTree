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
	case eTeam::None:
		break;
	case eTeam::Red:
		m_hRenderBrush = CreateSolidBrush(RGB(100, 20, 20));
		break;
	case eTeam::Blue:
		m_hRenderBrush = CreateSolidBrush(RGB(20, 20, 100));
		break;
	}

	SetRenderPrivateSize(8);

	SetUpdateTime(250);
}

void CEntity::Init(st_Pos& spawnpos)
{
	SetPos(spawnpos);
	m_eState = eState::Spawn;

	m_ai.SetTarget(this);
}

void CEntity::Update()
{
	m_ai.Update();
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
	ChangeState(eState::Idle);
	m_ai.SetMap(g_SimulatorManager->GetMap());
	m_ai.SetTarget(this);
}

bool CEntity::GetMoveNextPosInfo(st_PosInfo& posinfo)
{
	if (m_queueMovePath.empty())
	{
		if (!NetMovePath())
			return eBTresult::Failed;
	}
	posinfo = m_queueMovePath.front();

	return eBTresult::Success;
}

void CEntity::InitAstarPathtoMovePath()
{
	st_Pos nodePos = m_vecAstarPath.back();
	m_vecAstarPath.pop_back();

	m_ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
}

void CEntity::ChangeState(eState state)
{
	m_ai.ResetRuntime();

	m_eState = state;
}

bool CEntity::NetMovePath()
{
	if (m_vecAstarPath.empty() || !m_queueMovePath.empty())
		return eBTresult::Failed;

	st_Pos nodePos = m_vecAstarPath.back();
	m_vecAstarPath.pop_back();

	m_ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
	if (m_queueMovePath.empty())
		return eBTresult::Failed;

	return eBTresult::Success;
}

int CEntity::Move()
{
	if (m_vecAstarPath.empty() && m_queueMovePath.empty())
		return eBTresult::Success;

	if (m_queueMovePath.empty())
	{
		st_Pos nodePos = m_vecAstarPath.back();
		m_vecAstarPath.pop_back();

		m_ai.GetMap()->BresenhamLine(m_stPos.X, m_stPos.Y, nodePos.X, nodePos.Y, m_queueMovePath);
		if (m_queueMovePath.empty())
			return eBTresult::Failed;
	}

	st_Pos OldPos = m_stPos;

	SetPos(m_queueMovePath.front().pos);

	int oldRegionIndex = m_ai.GetMap()->GetRegionIndex(OldPos);
	m_iCurrentRegionIndex = m_ai.GetMap()->GetRegionIndex(m_stPos);

	if (oldRegionIndex != m_iCurrentRegionIndex)
	{
		CRegion* pOldRegion = m_ai.GetMap()->GetRegion(oldRegionIndex);
		if (pOldRegion)
			pOldRegion->ExitRegion(m_eTeam, this);
		CRegion* pNewRegion = m_ai.GetMap()->GetRegion(m_iCurrentRegionIndex);
		if (pNewRegion)
		{
			pNewRegion->EnterRegion(m_eTeam, this);
			if (pNewRegion->GetOccupyTeam() != m_eTeam && pNewRegion->GetOccupyTeam() != eTeam::None)
			{
				m_ai.GetTeam()->OnOccupyRegion(m_iCurrentRegionIndex, pNewRegion->GetOccupyTeam());
			}
		}

        // BT 규칙 검토: 목표나 점령 팀에 관계없이 어느 Region 진입에도 Occupy로 전환한다.
		if (m_iCurrentRegionIndex != -1)
		{
			ChangeState(eState::Occupy);
			m_ai.SetOccupyRegionIndex(m_iCurrentRegionIndex);
		}
	}

	m_queueMovePath.pop();

	return eBTresult::Running;
}
