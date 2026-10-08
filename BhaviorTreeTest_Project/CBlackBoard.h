#pragma once

#include <atomic>
#include "Defines.h"
#include <unordered_map>

class CEntity;
class CMap;
class CTeam;
class CBTNode;

struct st_RumtimeNodeInfo
{
	int m_iRet;
	int m_iCurrentIndex;
};

struct st_BlackBoard
{
	CEntity* m_pEntity;
	CMap* m_pMap;
	CTeam* m_pTeam;

	std::unordered_map<CBTNode*, st_RumtimeNodeInfo> m_mapRuntime;
	
	eAstarTargetType m_eAstarTargetType;
	int m_iTargetRegionIndex;
	int m_iOccupyRegionIndex;
	st_Pos m_stRegionCenterPos;
	st_Pos m_stRegionInsidePos;

	st_Pos m_stGoalPos;
	void ResetRuntime()
	{
		m_mapRuntime.clear();
	}

	void PushRuntime(CBTNode* pNode, const st_RumtimeNodeInfo& info)
	{
		m_mapRuntime[pNode] = info;
	}
};
