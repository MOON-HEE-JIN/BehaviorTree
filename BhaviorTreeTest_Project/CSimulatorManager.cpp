#include "CSimulatorManager.h"
#include <algorithm>

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

	m_updateTimerWheel.Push(update);

	m_vecRenderObject.push_back(m_pMap);

	CTeam* pTeam = new CTeam(eTeam::None, 0);
	m_vecTeam.push_back(pTeam);
	CreateEntity(eTeam::Red, 1);
	CreateEntity(eTeam::Blue, 1);

	CreateScarecrow();
}

CSimulatorManager::~CSimulatorManager()
{
	delete m_pMap;
}

void CSimulatorManager::NewUpdatePushTimerWheel(TWHEEL* pTweel)
{
	m_updateTimerWheel.Push(pTweel);
}

void CSimulatorManager::Update()
{
	m_updateTimerWheel.Update();

	int nLoop = m_updateTimerWheel.GetCompleteDataCount();
	const auto& vec = m_updateTimerWheel.GetCompleteData();
	for (int i = 0; i < nLoop; i++)
	{
		TWHEEL* pTwheel = vec[i];
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
			m_updateTimerWheel.Push(pTwheel);

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

	m_updateTimerWheel.ClearCompleteData();

	if (m_pMap->AllRegionOccupy())
	{
		printf("\n=== End Simulator ===\n");
	}
}

void CSimulatorManager::Render(HDC hdc)
{
	int nLoop = m_vecRenderObject.size();
	for (int i = 0; i < nLoop; i++)
	{
		m_vecRenderObject[i]->Render_Layer_01(hdc);
	}
	for (int i = 0; i < nLoop; i++)
	{
		m_vecRenderObject[i]->Render(hdc);
	}
}

void CSimulatorManager::CreateMap()
{
	m_pMap = new CMap;

	TWHEEL* pMapTweel = new TWHEEL;
	pMapTweel->TotalTimer = 1000;
	pMapTweel->pObj = m_pMap;
	pMapTweel->eUpdateType = eTUpdateType::Object;
	m_updateTimerWheel.Push(pMapTweel);

	const std::vector<CRegion*>& vec = m_pMap->GetRegionVector();
	int nLoop = vec.size();

	for (int i = 0; i < nLoop; i++)
	{
		// Region Update 등록
		TWHEEL* tweel = new TWHEEL;
		tweel->TotalTimer = vec[i]->GetUpdateTime();
		tweel->pObj = vec[i];
		tweel->eUpdateType = eTUpdateType::Object;

		m_updateTimerWheel.Push(tweel);
	}
}

void CSimulatorManager::CreateEntity(eTeam team, int count)
{
	CTeam* pTeam = new CTeam(team, m_pMap->GetRegionCount());
	m_vecTeam.push_back(pTeam);

	for (int i = 0; i < count; i++)
	{
		CEntity* pEntity = new CEntity(team, m_iEntityId++);
		st_Pos SpawnPos = m_pMap->GetSpawnPos((int)team);

		pEntity->Init(SpawnPos);
		pEntity->SetTeam(pTeam);

		TWHEEL* pTweel = new TWHEEL;
		pTweel->TotalTimer = pEntity->GetUpdateTime();
		pTweel->pObj = pEntity;
		pTweel->eUpdateType = eTUpdateType::Object;

		m_updateTimerWheel.Push(pTweel);

		m_vecRenderObject.push_back(pEntity);
		m_vecEntity.push_back(pEntity);
		m_mapTeamEntity[team].push_back(pEntity);
	}
}

void CSimulatorManager::CreateScarecrow()
{
	int d = 4;
	for (int i = 0; i < 3; i++)
	{
		CEntity* pScarecrow = new CEntity(eTeam::Blue, m_iEntityId++);
	
		st_Pos pos = m_mapTeamEntity[eTeam::Red][0]->GetPos();
		pos.X += d + i;

		pScarecrow->Init(pos);

		m_vecRenderObject.push_back(pScarecrow);
		m_vecEntity.push_back(pScarecrow);
		m_mapTeamEntity[eTeam::Blue].push_back(pScarecrow);
		m_vecScarecrow.push_back(pScarecrow);
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

	CEntity* pEntity = m_vecEntity[id];
	return pEntity->GetId() == id ? pEntity : nullptr;
}

int CSimulatorManager::FindNearEnemy(CEntity* pEntity, std::vector<CEntity*>& vec)
{
	vec.clear();
	eTeam team = pEntity->GetTeam() == eTeam::Red ? eTeam::Blue : eTeam::Red;

	if (team == eTeam::None)
		return 0;

	std::vector<CEntity*>& v = m_mapTeamEntity[team];
	int loop = v.size();

	st_Pos pos = pEntity->GetPos();
	int radius = pEntity->GetDetectionRadius();
	
	int dd = radius * radius;

	for (int i = 0; i < loop; i++)
	{
		CEntity* pEnemy = v[i];

		int dist = pos.LengthLength(pEnemy->GetPos());
		if (dd < dist)
			continue;

		vec.push_back(pEnemy);
	}
	std::sort(vec.begin(), vec.end(),
		[&pos](const CEntity* pA, const CEntity* pB)
	{
			int dist1 = pos.LengthLength(pA->GetPos());
			int dist2 = pos.LengthLength(pB->GetPos());
			return dist1 < dist2;
	});
#ifdef __DEBUG__
	for (auto p : vec)
	{
		int dist = pos.LengthLength(p->GetPos());
		printf("FindNearEnemy vec dist %d\n", dist);
	}
#endif // __DEBUG__

	return vec.size();
}
