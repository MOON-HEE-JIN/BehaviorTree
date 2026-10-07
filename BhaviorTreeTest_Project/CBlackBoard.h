#pragma once

#include <atomic>

class CEntity;
class CMap;
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
	st_RumtimeNodeInfo m_stRuntime;

	int m_iTargetRegionIndex;
};
