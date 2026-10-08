#include "CTeam.h"

CTeam::CTeam(eTeam team, int regioncount)
	:m_iMemberCount(0), m_iAliveCount(0)
{
	m_eTeam = team;
	m_vecRegionOccupy.resize(regioncount, eTeam::None);

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
