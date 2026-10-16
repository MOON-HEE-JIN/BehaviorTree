#pragma once

#include "CBehaviorTree.h"

class CBehaviorManager
{
public:
	CBehaviorManager();

public:
	CBehaviorTree* GetIdleTree() { return &m_idleTree; }
	CBehaviorTree* GetCombatTree() { return &m_combatTree; }
	CBehaviorTree* GetOccupyTree() { return &m_occupyTree; }

private:
	void CreateEntityIdleTree();
	void CreateEntityCombatTree();
	void CreateEntityOccupyTree();

private:
	CBehaviorTree m_idleTree;
	CBehaviorTree m_combatTree;
	CBehaviorTree m_occupyTree;

private:
	CBT_MoveSelector* CreateMoveSelector();
	CBT_MoveSelector* CreateCombatMoveSelector();
	CBT_Sequence* CreateAttackSequence(CBT_Condition* pSetTarget);
	CBT_Sequence* CreateOccupyAttackSequence();
};

extern CBehaviorManager g_CBehaviorManager;
