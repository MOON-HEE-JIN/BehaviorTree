#pragma once

#include "Defines.h"
#include <unordered_map>
#include <vector>
#include <cstdint>

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

	eAstarTargetType m_eAstarTargetType = eAstarTargetType::None;
	int m_iTargetRegionIndex = -1;
	int m_iOccupyRegionIndex = -1;
	int m_iEnemyId = -1;
	int m_reservedRegion = -1;
	std::uint64_t m_runtimeGeneration = 0;

	st_Pos m_stRegionCenterPos;
	st_Pos m_stRegionInsidePos;
	st_Pos m_stEnemyPos;

	st_Pos m_stGoalPos;
	void ResetRuntime()
	{
		++m_runtimeGeneration;
		m_mapRuntime.clear();
	}

	void PushRuntime(CBTNode* pNode, const st_RuntimeNodeInfo& info)
	{
		m_mapRuntime[pNode] = info;
	}
};
