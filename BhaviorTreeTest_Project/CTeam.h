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
	std::vector<int> m_reservations;

public:
	eTeam GetTeam() const { return m_eTeam; }
	void GetEmptyRegion(std::vector<int>& vec) const;

	int GetReservationCount(int index) const;
	void ChangeReservation(int index, int delta);
	void AddMember() { m_iMemberCount++; m_iAliveCount++; }
	void OnOccupyRegion(int regionindex, eTeam team);
};
