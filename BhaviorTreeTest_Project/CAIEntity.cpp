#include "CAIEntity.h"

#include "CSimulatorManager.h"
#include "CEntity.h"
#include "CBehaviorManager.h"

void CAIEntity::Update()
{
	switch (m_board.m_pEntity->GetState())
	{
	case eState::SPAWN:
	{
		CTeam* pTeam = g_SimulatorManager->GetTeam(m_board.m_pEntity->GetTeam());
		
		if (pTeam == nullptr)
			break;

		pTeam->AddMember();
		
		m_board.m_pEntity->OnSpawn();
		printf("%s Add Entity \n", pTeam->GetTeam() == eTeam::RED ? "Red" : "Blue");
	}
		break;
	case eState::IDLE:
		g_CBehaviorManager.GetIdleTree()->Run(&m_board);
		break;
	case eState::COMBAT:
		break;
	case eState::OCCUPY:
		g_CBehaviorManager.GetOccupyTree()->Run(&m_board);
		break;
	}
}
