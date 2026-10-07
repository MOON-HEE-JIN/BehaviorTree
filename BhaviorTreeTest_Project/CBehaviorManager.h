#pragma once

#include "CBehaviorTree.h"

class CBehaviorManager
{
public:
	CBehaviorManager();

public:
	CBehaviorTree* GetIdleTree() { return &m_IdleTree; }
	CBehaviorTree* GetCombatTree() { return &m_CombatTree; }
	CBehaviorTree* GetOccupyTree() { return &m_OccupyTree; }

private:
	void CreateEntityIdleTree();
	void CreateEntityCombatTree();
	void CreateEntityOccupyTree();

private:
	CBehaviorTree m_IdleTree;
	CBehaviorTree m_CombatTree;
	CBehaviorTree m_OccupyTree;
};

extern CBehaviorManager g_CBehaviorManager;