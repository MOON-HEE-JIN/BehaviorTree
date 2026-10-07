#include "CTeam.h"

CTeam::CTeam(eTeam team)
	:m_iMemberCount(0), m_iAliveCount(0), m_iDeadCount(0), m_iKillCount(0), m_iScore(0)
{
	m_eTeam = team;
}
