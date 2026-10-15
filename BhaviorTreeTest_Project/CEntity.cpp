#include "CEntity.h"
#include <Windows.h>

#include <string>
#include <math.h>
#include <list>
#include "WinThread.h"

CEntity::CEntity(eTeam team, int id)
	:m_hDetectionBrush(Gdiplus::Color(100, 53, 53, 53))
	, m_hDetectionPen(Gdiplus::Color(0,0,0))
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
		m_hDetectionPen.SetColor(Gdiplus::Color(100, 20, 20));
		break;
	case eTeam::Blue:
		m_hRenderBrush = CreateSolidBrush(RGB(20, 20, 100));
		m_hDetectionPen.SetColor(Gdiplus::Color(20, 20, 100));
		break;
	}

	
	SetRenderPrivateSize(16);
	SetDetectionRadius(8);

	SetUpdateTime(250);
}

void CEntity::RenderCharacter(HDC hdc)
{
	SelectObject(hdc, m_hRenderBrush);
	Ellipse(hdc
		, m_stRenderPos.X - GetRenderPrivateHalfSize()
		, m_stRenderPos.Y - GetRenderPrivateHalfSize()
		, m_stRenderPos.X + GetRenderPrivateHalfSize()
		, m_stRenderPos.Y + GetRenderPrivateHalfSize());
}

void CEntity::RenderDetection(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);

	int X = m_stRenderPos.X - GetRenderDetectionRadiusSize();
	int Y = m_stRenderPos.Y - GetRenderDetectionRadiusSize();
	int Size = GetRenderDetectionRadiusSize() * 2;
	
	graphics.FillEllipse(&m_hDetectionBrush, X, Y, Size, Size);

	graphics.DrawEllipse(&m_hDetectionPen, X, Y, Size, Size);

	X = m_stRenderPos.X - GetRenderAttackRangeSize();
	Y = m_stRenderPos.Y - GetRenderAttackRangeSize();
	Size = GetRenderAttackRangeSize() * 2;
	graphics.DrawEllipse(&m_hDetectionPen, X, Y, Size, Size);
}

void CEntity::Init(st_Pos& spawnpos)
{
	SetPos(spawnpos);
	SetAttackRange(5);
	m_eState = eState::Spawn;
	
	m_ai.SetTarget(this);
}

void CEntity::Update()
{
	m_ai.Update();
}

void CEntity::Render(HDC hdc)
{
	if (!GetEnable())
		return;
	RenderCharacter(hdc);
}

void CEntity::Render_Layer_01(HDC hdc)
{
	if (!GetEnable())
		return;
	RenderDetection(hdc);
}

void CEntity::OnSpawn()
{
	ChangeState(eState::Idle);
	m_ai.SetMap(g_SimulatorManager->GetMap());
	m_ai.SetTarget(this);
}

int CEntity::OnAttack(int id)
{
	if (id < 0)
		return eBTresult::Failed;
	CEntity* pEnemy = g_SimulatorManager->GetEntity(id);
	if (pEnemy == nullptr)
		return eBTresult::Failed;

	pEnemy->OnDamage(GetId(), 10);

	return eBTresult::Success;
}

int CEntity::OnDamage(int id, int damage)
{
	int preHP = m_iHP;
	m_iHP -= damage;
	
	printf("Entity[%d] -attack-> Entity[%d][%d][%d]\n", id, GetId(), preHP, m_iHP);
	if (m_iHP <= 0)
		OnDead();
	return 0;
}

void CEntity::OnDead()
{
	SetEnable(false);
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

        if (m_iCurrentRegionIndex != -1)
		{
			CRegion* pNewRegion = m_ai.GetMap()->GetRegion(m_iCurrentRegionIndex);
			if (pNewRegion)
			{
				if (pNewRegion->GetOccupyTeam() != m_eTeam)
				{
					ChangeState(eState::Occupy);
					m_ai.SetOccupyRegionIndex(m_iCurrentRegionIndex);
				}
			}	
		}

	}

	m_queueMovePath.pop();

	return eBTresult::Running;
}
