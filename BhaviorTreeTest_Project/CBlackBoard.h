#pragma once

#include "Defines.h"
#include <unordered_map>

class CEntity;
class CMap;
class CTeam;
class CBTNode;

struct st_RuntimeNodeInfo
{
	int m_iCurrentIndex = 0;
};

struct st_BlackBoard
{
	CEntity* m_pEntity = nullptr;
	CMap* m_pMap = nullptr;
	CTeam* m_pTeam = nullptr;

	std::unordered_map<CBTNode*, st_RuntimeNodeInfo> m_mapRuntime;
	std::vector<CEntity*> m_vecDetectionEnemy;
	std::vector<CEntity*> m_vecDetectionRegionEnemy;

	eAstarTargetType m_eAstarTargetType = eAstarTargetType::None;
	int m_iTargetRegionIndex = -1;
	int m_iOccupyRegionIndex = -1;
	int m_iEnemyId = -1;

	st_Pos m_stRegionCenterPos;
	st_Pos m_stRegionInsidePos;
	st_Pos m_stEnemyPos;

	int m_iRegionCostIndex = -1;


	st_Pos m_stGoalPos;
	void ResetRuntime()
	{
		m_mapRuntime.clear();
	}

	void PushRuntime(CBTNode* pNode, const st_RuntimeNodeInfo& info)
	{
		m_mapRuntime[pNode] = info;
	}
};
