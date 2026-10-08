#include "CSimulatorManager.h"

CSimulatorManager* g_SimulatorManager;

CSimulatorManager::CSimulatorManager()
{
	m_iEntityId = 0;
	
	SetUpdateTime( 10 * 60 * 1 * 1000);

	CreateMap();

	TWHEEL* update = new TWHEEL;
	update->TotalTimer = GetUpdateTime();
	update->pObj = this;
	update->eUpdateType = eTUpdateType::End;

	m_cUpdateTimerWheel.Push(update);

	m_vecRenderObject.push_back((WinObject*)m_cMap);

	CreateEntity(eTeam::RED, 1);
	CreateEntity(eTeam::BLUE, 1);
}

CSimulatorManager::~CSimulatorManager()
{
	delete m_cMap;
}

void CSimulatorManager::NewUpdatePushTimerWheel(TWHEEL* pTweel)
{
	m_cUpdateTimerWheel.Push(pTweel);
}

void CSimulatorManager::Update()
{
	m_cUpdateTimerWheel.Update();

	int nLoop = m_cUpdateTimerWheel.m_vecCompleteData.size();
	std::vector<TWHEEL*>* vec = &m_cUpdateTimerWheel.m_vecCompleteData;
	for (int i = 0; i < nLoop; i++)
	{
		TWHEEL* pTwheel = (*vec)[i];
		WinObject* pObject = pTwheel->pObj;

		switch (pTwheel->eUpdateType)
		{
		case eTUpdateType::Object:
			pObject->Update();

			if (pObject->GetUpdateTime() == 0)
			{
				delete pTwheel;
				break;
			}
			pTwheel->TotalTimer = pObject->GetUpdateTime();
			m_cUpdateTimerWheel.Push(pTwheel);

			break;
		case eTUpdateType::CoolTime:
			pObject->CheckCoolTime(pTwheel->iSkillIndex);
			delete pTwheel;
			break;
		case eTUpdateType::DuringTime:
			pObject->CheckDuringTime(pTwheel->iSkillIndex);
			delete pTwheel;
			break;
		case eTUpdateType::End:
			break;
		}

	}

	m_cUpdateTimerWheel.m_vecCompleteData.clear();

	if (m_cMap->AllRegionOccupy())
	{
		printf("\n=== End Simulator ===\n");
	}
}

void CSimulatorManager::Render(HDC hdc)
{
	int nLoop = m_vecRenderObject.size();
	for (int i = 0; i < nLoop; i++)
	{
		m_vecRenderObject[i]->Render(hdc);
	}
}

void CSimulatorManager::CreateMap()
{
	m_cMap = new CMap;

	TWHEEL* pMapTweel = new TWHEEL;
	pMapTweel->TotalTimer = 1000;
	pMapTweel->pObj = m_cMap;
	pMapTweel->eUpdateType = eTUpdateType::Object;
	m_cUpdateTimerWheel.Push(pMapTweel);

	const std::vector<CRegion*> vec = m_cMap->GetRegionVector();
	int nLoop = vec.size();

	for (int i = 0; i < nLoop; i++)
	{
		// Region Update 등록
		TWHEEL* tweel = new TWHEEL;
		tweel->TotalTimer = vec[i]->GetUpdateTime();
		tweel->pObj = vec[i];
		tweel->eUpdateType = eTUpdateType::Object;

		m_cUpdateTimerWheel.Push(tweel);
	}
}

void CSimulatorManager::CreateEntity(eTeam team, int count)
{
	CTeam* pTeam = new CTeam(team, m_cMap->GetRegionCount());
	m_vecTeam.push_back(pTeam);

	for (int i = 0; i < count; i++)
	{
		CEntity* pEntity = new CEntity(team, m_iEntityId++);
		st_Pos SpawnPos = m_cMap->GetSpawnPos((int)team);

		pEntity->Init(SpawnPos);
		pEntity->SetTeam(pTeam);

		TWHEEL* pTweel = new TWHEEL;
		pTweel->TotalTimer = pEntity->GetUpdateTime();
		pTweel->pObj = pEntity;
		pTweel->eUpdateType = eTUpdateType::Object;

		m_cUpdateTimerWheel.Push(pTweel);

		m_vecRenderObject.push_back(pEntity);
		m_vecEntity.push_back(pEntity);
	}
}

CTeam* CSimulatorManager::GetTeam(eTeam team)
{
	for(CTeam* pTeam : m_vecTeam)
	{
		if (pTeam->GetTeam() == team)
			return pTeam;
	}
	return nullptr;
}

CEntity* CSimulatorManager::GetEntity(int id)
{
	if(id < 0 || id >= (int)m_vecEntity.size())
		return nullptr;

	return m_vecEntity[id];
}
