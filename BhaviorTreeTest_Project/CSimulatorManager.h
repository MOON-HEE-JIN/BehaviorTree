#pragma once

#include "CMap.h"
#include "CEntity.h"
#include "CTimerWheelManager.h"
#include "CTeam.h"

#include <unordered_map>
#include <list>

class CSimulatorManager : public WinObject
{
public:
	CSimulatorManager();
	~CSimulatorManager();

private:
	CMap* m_pMap;
	std::vector<WinObject*> m_vecRenderObject;
	std::vector<CTeam*> m_vecTeam;
	std::vector<CEntity*> m_vecEntity;
	std::unordered_map<eTeam, std::vector<CEntity*>> m_mapTeamEntity;
	std::vector<CEntity*> m_vecScarecrow;

	CTimerWheelManager m_updateTimerWheel;

	int m_iEntityId;
public:
	void NewUpdatePushTimerWheel(TWHEEL* pTweel);
	void NotifyOccupyRegion(int index, eTeam team);
	// WinObject을(를) 통해 상속됨
	void Update() override;
	void Render(HDC hdc) override;

	void CreateMap();
	void CreateEntity(eTeam team, int count);
	void CreateScarecrow();

	CTeam* GetTeam(eTeam team);
	CMap* GetMap() const { return m_pMap; }
	CEntity* GetEntity(int id);

	int FindNearEnemy(CEntity* pEntity, std::vector<CEntity*>& vec);
};

extern CSimulatorManager* g_SimulatorManager;
