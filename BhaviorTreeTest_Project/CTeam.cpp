#include "CTeam.h"

CTeam::CTeam(eTeam team, int regioncount)
	:m_iMemberCount(0), m_iAliveCount(0)
{
	m_eTeam = team;

	m_vecRegionOccupy.resize(regioncount, eTeam::None);
	m_reservations.resize(regioncount, 0);

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

int CTeam::GetReservationCount(int index) const
{
    return index >= 0 && index < static_cast<int>(m_reservations.size()) ? m_reservations[index] : 0;
}

void CTeam::ChangeReservation(int index, int delta)
{
    if (index < 0 || index >= static_cast<int>(m_reservations.size()))
		return;
    
	int count = m_reservations[index];
    count += delta;
    if (count < 0) count = 0;
}
