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
	std::vector<st_RegionCost> m_vecRegionOccupyCost;

	int m_iMemberCount;
	int m_iAliveCount;

public:
	void OnOccupyRegion(int regionindex, eTeam team);

	void AddMember() { m_iMemberCount++; m_iAliveCount++; }
	void AddRegionOccupyCost(int regionindex, bool b = true);

	eTeam GetTeam() const { return m_eTeam; }
	void GetEmptyRegion(std::vector<int>& vec) const;
	void GetOtherRegion(std::vector<int>& vec) const;
	int GetMinCostRegion();
};
