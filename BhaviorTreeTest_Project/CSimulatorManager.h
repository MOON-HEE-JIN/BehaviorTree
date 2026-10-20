#pragma once

#include "CMap.h"
#include "CEntity.h"
#include "CTimerWheelManager.h"
#include "CTeam.h"

#include <unordered_map>
#include <list>
#include <memory>
#include <mutex>

class CSimulatorManager : public WinObject
{
public:
    enum class EndReason { None, AllRegionsCaptured, NoLivingEntities };
    struct Result { EndReason reason = EndReason::None; int red = 0; int blue = 0; eTeam winner = eTeam::None; };
    
	explicit CSimulatorManager(int redCount = 5, int blueCount = 5, bool scarecrows = false);
    bool IsFinished() const { return m_result.reason != EndReason::None; }
    const Result& GetResult() const { return m_result; }
    void CheckEndCondition();
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
    Result m_result;
    std::recursive_mutex m_stateMutex;
    std::unordered_map<TWHEEL*, std::unique_ptr<TWHEEL>> m_events;
    void Finish(EndReason reason);
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
