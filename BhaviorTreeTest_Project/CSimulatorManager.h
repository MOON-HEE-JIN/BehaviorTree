#pragma once

#include "CMap.h"
#include "CEntity.h"
#include "CTimerWheelManager.h"
#include "CTeam.h"

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

	CTeam* GetTeam(eTeam team);
	CMap* GetMap() const { return m_pMap; }
	CEntity* GetEntity(int id);
};

extern CSimulatorManager* g_SimulatorManager;
