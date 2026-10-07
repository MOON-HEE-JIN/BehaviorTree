#include "CTeam.h"

CTeam::CTeam(eTeam team, int regioncount)
	:m_iMemberCount(0), m_iAliveCount(0), m_iDeadCount(0), m_iKillCount(0), m_iScore(0)
{
	m_eTeam = team;
	m_vecRegionOccupy.resize(regioncount, eTeam::NONE);

}

void CTeam::GetEmptyRegion(std::vector<int>& vec)
{
	for (size_t i = 0; i < m_vecRegionOccupy.size(); ++i)
	{
		if (m_vecRegionOccupy[i] == eTeam::NONE)
		{
			vec.push_back((int)i);
		}
	}
}
