#pragma once

#include "CAI.h"

class CEntity;
class CMap;
class CTeam;

class CAIEntity : public CAIController
{
public:
	void Update();

	void SetTeam(CTeam* pTeam) { m_blackBoard.m_pTeam = pTeam; }
	void SetTarget(CEntity* pTarget) { m_blackBoard.m_pEntity = pTarget; }
	void SetMap(CMap* pMap) { m_blackBoard.m_pMap = pMap; }

	CMap* GetMap() const { return m_blackBoard.m_pMap; }
    CTeam* GetTeam() const { return m_blackBoard.m_pTeam; }

    void ResetRuntime() { m_blackBoard.ResetRuntime(); }
    void CancelAction();
    void ReleaseReservation();
    void ReserveRegion(int index);
    void SetOccupyRegionIndex(int index) { m_blackBoard.m_iOccupyRegionIndex = index; }
};
