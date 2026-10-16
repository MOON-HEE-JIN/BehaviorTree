#include "CTeam.h"
#include <algorithm>

CTeam::CTeam(eTeam team, int regioncount)
	:m_iMemberCount(0), m_iAliveCount(0)
{
	m_eTeam = team;
	m_vecRegionOccupy.resize(regioncount, eTeam::None);
	m_vecRegionOccupyCost.resize(regioncount);

	for (int i = 0; i < regioncount; i++)
		m_vecRegionOccupyCost[i] = {i, 0};
	
}

void CTeam::GetEmptyRegion(std::vector<int>& vec) const
{
	vec.reserve(vec.size() + m_vecRegionOccupy.size());
	for (size_t i = 0; i < m_vecRegionOccupy.size(); ++i)
	{
		if (m_vecRegionOccupy[i] == eTeam::None)
		{
			vec.push_back((int)i);
		}
	}
}

void CTeam::GetOtherRegion(std::vector<int>& vec) const
{
	vec.reserve(vec.size() + m_vecRegionOccupy.size());
	for (size_t i = 0; i < m_vecRegionOccupy.size(); ++i)
	{
		if (m_vecRegionOccupy[i] != m_eTeam)
		{
			vec.push_back((int)i);
		}
	}
}

int CTeam::GetMinCostRegion()
{
	return m_vecRegionOccupyCost[0].index;
}

void CTeam::OnOccupyRegion(int regionindex, eTeam team)
{
	if (regionindex >= m_vecRegionOccupy.size())
		return;
	eTeam preTem = m_vecRegionOccupy[regionindex];
	m_vecRegionOccupy[regionindex] = team;

	if (team == m_eTeam)
	{
		for (auto& cost : m_vecRegionOccupyCost)
		{
			if (cost.index == regionindex)
				cost.cost += 100;
		}
	}
	else
	{
		if (preTem == m_eTeam)
		{
			for (auto& cost : m_vecRegionOccupyCost)
			{
				if (cost.index == regionindex)
					cost.cost -= 100;
			}
		}
	}
}

void CTeam::AddRegionOccupyCost(int regionindex, int _cost)
{
	for (auto& cost : m_vecRegionOccupyCost)
	{
		if (cost.index == regionindex)
			cost.cost += _cost;
	}

	std::sort(m_vecRegionOccupyCost.begin(), m_vecRegionOccupyCost.end(),
		[](const st_RegionCost& A, const st_RegionCost& B)
		{
			return A.cost < B.cost;
		});
}
