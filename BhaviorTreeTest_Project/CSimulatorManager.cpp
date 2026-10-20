#include "CSimulatorManager.h"
#include <algorithm>

CSimulatorManager* g_SimulatorManager;

CSimulatorManager::CSimulatorManager(int redCount, int blueCount, bool scarecrows)
{
	m_iEntityId = 0;

	CreateMap();

	m_vecRenderObject.push_back(m_pMap);

	CTeam* pTeam = new CTeam(eTeam::None, 0);
	m_vecTeam.push_back(pTeam);

	CreateEntity(eTeam::Red, redCount);
    CreateEntity(eTeam::Blue, blueCount);

	SetUpdateTime(2 * 1000);
	TWHEEL* pTweel = new TWHEEL;
	pTweel->TotalTimer = 2 * 1000;
	pTweel->pObj = this;
	pTweel->eUpdateType = eTUpdateType::Event;

	NewUpdatePushTimerWheel(pTweel);
    // if (scarecrows) CreateScarecrow();
}

CSimulatorManager::~CSimulatorManager()
{
    m_events.clear();
    for (auto* entity : m_vecEntity) delete entity;
    for (auto* team : m_vecTeam) delete team;
    delete m_pMap;
}


void CSimulatorManager::NewUpdatePushTimerWheel(TWHEEL* event)
{
    if (!event) return;
    
	if (m_events.count(event)) return;
    
	m_events.emplace(event, std::unique_ptr<TWHEEL>(event));
    if (IsFinished())
	{
		m_events.erase(event);
		return;
	}
    m_updateTimerWheel.Push(event);
}

void CSimulatorManager::NotifyOccupyRegion(int index, eTeam team)
{
	int loop = m_vecTeam.size();
	for (int i = 0; i < loop; i++)
	{
		m_vecTeam[i]->OnOccupyRegion(index, team);
	}
}

void CSimulatorManager::Update()
{
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
    
	if (IsFinished())
		return;
    
	m_updateTimerWheel.Update();
    
	const auto& events = m_updateTimerWheel.GetCompleteData();
    for (TWHEEL* event : events)
    {
        if (IsFinished())
		{
			m_events.erase(event);
			continue;
		}
        
		WinObject* object = event->pObj;

        if (!object || !object->GetEnable()) { m_events.erase(event); continue; }
        switch (event->eUpdateType)
        {
        case eTUpdateType::Object: object->Update(); break;
        case eTUpdateType::CoolTime: object->CheckCoolTime(event->iSkillIndex); break;
        case eTUpdateType::DuringTime: object->CheckDuringTime(event->iSkillIndex); break;
		case eTUpdateType::Event: CheckEndCondition(); break;
        default: break;
        }

		if (!IsFinished() &&
			event->eUpdateType == eTUpdateType::Object &&
			object->GetEnable() &&
			object->GetUpdateTime() > 0)
        {
            event->TotalTimer = object->GetUpdateTime();
            m_updateTimerWheel.Push(event);
        }
        else m_events.erase(event);
    }
    m_updateTimerWheel.ClearCompleteData();
}

void CSimulatorManager::Render(HDC hdc)
{
    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
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

	NewUpdatePushTimerWheel(pMapTweel);

	const std::vector<CRegion*>& vec = m_pMap->GetRegionVector();
	int nLoop = vec.size();

	for (int i = 0; i < nLoop; i++)
	{
		// Region Update 등록
		TWHEEL* tweel = new TWHEEL;
		tweel->TotalTimer = vec[i]->GetUpdateTime();
		tweel->pObj = vec[i];
		tweel->eUpdateType = eTUpdateType::Object;

		NewUpdatePushTimerWheel(tweel);
	}
}

void CSimulatorManager::CreateEntity(eTeam team, int count)
{
    CTeam* pTeam = GetTeam(team);
    if (!pTeam) 
	{ 
		pTeam = new CTeam(team, m_pMap->GetRegionCount()); m_vecTeam.push_back(pTeam);
	}

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

		NewUpdatePushTimerWheel(pTweel);

		m_vecRenderObject.push_back(pEntity);
		m_vecEntity.push_back(pEntity);
		m_mapTeamEntity[team].push_back(pEntity);
	}
}

void CSimulatorManager::CreateScarecrow()
{
// 허수아비 테스트용 코드
	if (m_mapTeamEntity[eTeam::Red].empty() || IsFinished()) return;
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

	if (!pEntity->GetEnable() || pEntity->GetState() == eState::Dead)
		return nullptr;

	return pEntity->GetId() == id ? pEntity : nullptr;
}

int CSimulatorManager::FindNearEnemy(CEntity* pEntity, std::vector<CEntity*>& vec)
{
	vec.clear();

	if (pEntity->GetTeam() == eTeam::None)
		return 0;
	
	eTeam team = pEntity->GetTeam() == eTeam::Red ? eTeam::Blue : eTeam::Red;

	std::vector<CEntity*>& v = m_mapTeamEntity[team];
	int loop = v.size();

	st_Pos pos = pEntity->GetPos();
	int radius = pEntity->GetDetectionRadius();
	
	int dd = radius * radius;

	for (int i = 0; i < loop; i++)
	{
		CEntity* pEnemy = v[i];

		if (!pEnemy->GetEnable())
			continue;

		int dist = pos.LengthLength(pEnemy->GetPos());
        
		if (dd < dist &&
			(pEntity->GetCurrentRegionIndex() < 0 || pEntity->GetCurrentRegionIndex() != pEnemy->GetCurrentRegionIndex()))
			continue;

		vec.push_back(pEnemy);
	}
	std::sort(vec.begin(), vec.end(),
		[&pos](const CEntity* pA, const CEntity* pB)
		{
			int dist1 = pos.LengthLength(pA->GetPos());
			int dist2 = pos.LengthLength(pB->GetPos());
			
			return dist1 == dist2 ? pA->GetId() < pB->GetId() : dist1 < dist2;
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

void CSimulatorManager::CheckEndCondition()
{
    if (IsFinished())
		return;

    if (m_pMap->AllRegionOccupy())
	{ 
		Finish(EndReason::AllRegionsCaptured);
		return;
	}

    const bool alive = std::any_of(m_vecEntity.begin(), m_vecEntity.end(),
		[](const CEntity* entity) {
        return entity->GetEnable() && entity->GetState() != eState::Dead && entity->GetState() != eState::End;
    });

    if (!alive)
		Finish(EndReason::NoLivingEntities);
}

void CSimulatorManager::Finish(EndReason reason)
{
    if (IsFinished())
		return;
    m_result.reason = reason;

    for (auto* region : m_pMap->GetRegionVector())
    {
        if (region->GetOccupyTeam() == eTeam::Red)
			++m_result.red;
        else if (region->GetOccupyTeam() == eTeam::Blue)
			++m_result.blue;
    }

    m_result.winner = m_result.red == m_result.blue ? eTeam::None : m_result.red > m_result.blue ? eTeam::Red : eTeam::Blue;
	
	for (auto* entity : m_vecEntity)
	{
		if (entity->GetEnable()) entity->ChangeState(eState::End);
	}

	printf("End Simulator: reason=%d Red=%d Blue=%d winner=%d\n", static_cast<int>(reason), m_result.red, m_result.blue, static_cast<int>(m_result.winner));
}
