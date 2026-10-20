#include "CTeam.h"

CTeam::CTeam(eTeam team, int regioncount)
	:m_iMemberCount(0), m_iAliveCount(0)
{
	m_eTeam = team;

	m_vecRegionOccupy.resize(regioncount, eTeam::None);
	m_vecReservations.resize(regioncount, 0);
	m_vecCosts.resize(regioncount, 0);
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

void CTeam::OnOccupyRegion(int regionindex, eTeam team)
{
	if (regionindex >= m_vecRegionOccupy.size())
		return;

	m_vecRegionOccupy[regionindex] = team;
}

void CTeam::AddCost(int regionindex, int cost)
{
	if (regionindex >= m_vecRegionOccupy.size() || regionindex < 0)
		return;

	m_vecCosts[regionindex] += cost;
}

int CTeam::GetReservationCount(int index) const
{
    return index >= 0 && index < static_cast<int>(m_vecReservations.size()) ? m_vecReservations[index] : 0;
}

int CTeam::GetCost(int regionindex)
{
	return regionindex >= 0 && regionindex < static_cast<int>(m_vecCosts.size()) ? m_vecCosts[regionindex] : 0;
}

void CTeam::ChangeReservation(int index, int delta)
{
    if (index < 0 || index >= static_cast<int>(m_vecReservations.size()))
		return;
    
	int count = m_vecReservations[index];
    count += delta;
    if (count < 0) count = 0;
}
