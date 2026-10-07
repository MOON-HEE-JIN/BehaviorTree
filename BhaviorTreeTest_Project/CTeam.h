#pragma once

#include "Defines.h"
#include <vector>

class CTeam
{
public:
	CTeam(eTeam team, int regioncount);

private:
	eTeam m_eTeam;
	std::vector<eTeam> m_vecRegionOccupy;

	int m_iMemberCount;
	int m_iAliveCount;
	int m_iKillCount;
	int m_iDeadCount;

	int m_iScore;

public:
	eTeam GetTeam() { return m_eTeam; }
	void GetEmptyRegion(std::vector<int>& vec);

	void AddMember() { m_iMemberCount++; m_iAliveCount++; }
	void OnOccupyRegion(int regionindex, eTeam team) { m_vecRegionOccupy[regionindex] = team; }
};