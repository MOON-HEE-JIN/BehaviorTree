#include "CAIEntity.h"

#include "CSimulatorManager.h"
#include "CEntity.h"
#include "CBehaviorManager.h"

void CAIEntity::Update()
{
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
