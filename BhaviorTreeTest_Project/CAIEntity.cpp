#include "CAIEntity.h"

#include "CSimulatorManager.h"
#include "CEntity.h"
#include "CBehaviorManager.h"

void CAIEntity::Update()
{
    if (m_blackBoard.m_pMap && m_blackBoard.m_reservedRegion >= 0)
    {
        auto* region = m_blackBoard.m_pMap->GetRegion(m_blackBoard.m_reservedRegion);
        if (!region || region->GetOccupyTeam() == m_blackBoard.m_pEntity->GetTeam())
        {
            CancelAction();
            m_blackBoard.m_pEntity->ClearMovePath();
        }
    }
	switch (m_blackBoard.m_pEntity->GetState())
	{
	case eState::Spawn:
	{
		CTeam* pTeam = g_SimulatorManager->GetTeam(m_blackBoard.m_pEntity->GetTeam());

		if (pTeam == nullptr)
			break;

		pTeam->AddMember();

		m_blackBoard.m_pEntity->OnSpawn();
		printf("%s Add Entity \n", pTeam->GetTeam() == eTeam::Red ? "Red" : "Blue");
	}
		break;
	case eState::Idle:
		g_CBehaviorManager.GetIdleTree()->Run(&m_blackBoard);
		break;
	case eState::Combat:
		g_CBehaviorManager.GetCombatTree()->Run(&m_blackBoard);
		break;
	case eState::Occupy:
		g_CBehaviorManager.GetOccupyTree()->Run(&m_blackBoard);
		break;
	}
}

void CAIEntity::ReleaseReservation()
{
    if (m_blackBoard.m_pTeam && m_blackBoard.m_reservedRegion >= 0)
    {
        m_blackBoard.m_pTeam->ChangeReservation(m_blackBoard.m_reservedRegion, -1);
        m_blackBoard.m_pTeam->AddCost(m_blackBoard.m_reservedRegion, m_blackBoard.m_pMap->GetDiagonalLine());
    }
    m_blackBoard.m_reservedRegion = -1;
}

void CAIEntity::ReserveRegion(int index)
{
    if (index == m_blackBoard.m_reservedRegion) return;
    ReleaseReservation();
    if (m_blackBoard.m_pTeam && index >= 0)
    {
        m_blackBoard.m_pTeam->ChangeReservation(index, 1);
        m_blackBoard.m_pTeam->AddCost(index, m_blackBoard.m_pMap->GetDiagonalLine());
        m_blackBoard.m_reservedRegion = index;
    }
}

void CAIEntity::CancelAction()
{
    ReleaseReservation();
    m_blackBoard.ResetRuntime();
    m_blackBoard.m_iTargetRegionIndex = -1;
    m_blackBoard.m_iOccupyRegionIndex = -1;
    m_blackBoard.m_iEnemyId = -1;
    m_blackBoard.m_eAstarTargetType = eAstarTargetType::None;
    m_blackBoard.m_vecDetectionEnemy.clear();
}
