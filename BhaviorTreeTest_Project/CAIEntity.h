#pragma once

#include "CAI.h"

class CEntity;
class CMap;
class CTeam;

class CAIEntity : public CAIController
{
public:

public:
	void Update();

	void SetTeam(CTeam* pTeam) { m_board.m_pTeam = pTeam; }
	void SetTarget(CEntity* pTarget) { m_board.m_pEntity = pTarget; }
	void SetMap(CMap* pMap) { m_board.m_pMap = pMap; }

	CMap* GetMap() { return m_board.m_pMap; }
};