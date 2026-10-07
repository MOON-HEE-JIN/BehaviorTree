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
	CMap* m_cMap;
	std::vector<WinObject*> m_vecRenderObject;
	std::vector<CTeam*> m_vecTeam;
	std::vector<CEntity*> m_vecEntity;

	CTimerWheelManager m_cUpdateTimerWheel;

	int m_iEntityId;
public:
	void NewUpdatePushTimerWheel(TWHEEL* pTweel);
	// WinObject을(를) 통해 상속됨
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void CreateMap();
	void CreateEntity(eTeam team, int count);

	CTeam* GetTeam(eTeam team);
	CMap* GetMap() { return m_cMap; }
	CEntity* GetEntity(int id);
};

extern CSimulatorManager* g_SimulatorManager;