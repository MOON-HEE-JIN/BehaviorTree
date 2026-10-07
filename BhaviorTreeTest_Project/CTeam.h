#pragma once

#include "Defines.h"

class CTeam
{
public:
	CTeam(eTeam team);

private:
	eTeam m_eTeam;

	int m_iMemberCount;
	int m_iAliveCount;
	int m_iKillCount;
	int m_iDeadCount;

	int m_iScore;

public:
	eTeam GetTeam() { return m_eTeam; }
	void AddMember() { m_iMemberCount++; m_iAliveCount++; }
};