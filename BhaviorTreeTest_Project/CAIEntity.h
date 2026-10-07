#pragma once

#include "CAI.h"

class CEntity;
class CMap;

class CAIEntity : public CAIController
{
public:

public:
	void Update();

	void SetTarget(CEntity* pTarget) { m_board.m_pEntity = pTarget; }
	void SetMap(CMap* pMap) { m_board.m_pMap = pMap; }

	CMap* GetMap() { return m_board.m_pMap; }
};