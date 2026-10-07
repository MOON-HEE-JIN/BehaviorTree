#pragma once

#include <atomic>
#include "Defines.h"

class CEntity;
class CMap;
class CTeam;

struct st_RumtimeNodeInfo
{
	int m_iRet;
	int m_iCurrentIndex;
};

class CBlackBoard
{
public:
	CEntity* m_pEntity;
	CMap* m_pMap;
	CTeam* m_pTeam;

	st_RumtimeNodeInfo m_stRuntime;

	eAstarTargetType m_eAstarTargetType;
	int m_iTargetRegionIndex;
};
