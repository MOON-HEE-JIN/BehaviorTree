#include "CBehaviorManager.h"

CBehaviorManager g_CBehaviorManager;

CBehaviorManager::CBehaviorManager()
{
	CreateEntityIdleTree();
	CreateEntityOccupyTree();
	CreateEntityCombatTree();
}

void CBehaviorManager::CreateEntityIdleTree()
{

	CBT_Sequence* pSequenceRoot = new CBT_Sequence();

	m_idleTree.SetRoot(pSequenceRoot);
	{
		CBT_FindLowCostRegion* pMinCostRegion = new CBT_FindLowCostRegion();
		pSequenceRoot->Push(pMinCostRegion);
	}
	{
		CBT_FindPath* pNode = new CBT_FindPath();
		pSequenceRoot->Push(pNode);
	}
	{
		pSequenceRoot->Push(CreateMoveSelector());
	}
}

void CBehaviorManager::CreateEntityCombatTree()
{
	CBT_ConditionBranch* pBranchRoot = new CBT_ConditionBranch();
	m_combatTree.SetRoot(pBranchRoot);

	CBT_SetEnemyTarget* pSetTarget = new CBT_SetEnemyTarget();
	pBranchRoot->SetCondition(pSetTarget);

	{
		// eBTresult::Failed
		CBT_IsEnemyNearCombatTree* pNear = new CBT_IsEnemyNearCombatTree();
		pBranchRoot->Push(pNear);
	}
	{
		// eBTresult::Success
		CBT_Sequence* pSequence = new CBT_Sequence();
		CBT_SetEnemyTarget* pSetTarget = new CBT_SetEnemyTarget();
		pSequence->Push(pSetTarget);

		CBT_ConditionBranch* pAttackRoot = new CBT_ConditionBranch();
		CBT_IsAttack* pIsAttack = new CBT_IsAttack();
		pAttackRoot->SetCondition(pIsAttack);
		{
			// 공격 실패시 이동
			CBT_Sequence* pFSequence = new CBT_Sequence();
			CBT_FindPath* pFindPath = new CBT_FindPath();
			pFSequence->Push(pFindPath);
			pFSequence->Push(CreateCombatMoveSelector());
			pAttackRoot->Push(pFSequence);
		}
		{
			// 공격 판정 성공
			CBT_Attack* pAttack = new CBT_Attack();
			pAttackRoot->Push(pAttack);
		}
		pSequence->Push(pAttackRoot);

		pBranchRoot->Push(pSequence);
	}

}

void CBehaviorManager::CreateEntityOccupyTree()
{
	CBT_ConditionBranch* pBranchRoot = new CBT_ConditionBranch();
	m_occupyTree.SetRoot(pBranchRoot);

	CBT_CheckEnemyInRegion* pCheckEnemy = new CBT_CheckEnemyInRegion();
	pBranchRoot->SetCondition(pCheckEnemy);

	{
		CBT_Sequence* pSequenceNoEnemy = new CBT_Sequence();
		CBT_FindOccupyingRegionInPos* pFindOccupyRegion = new CBT_FindOccupyingRegionInPos();
		CBT_FindPath* pNode = new CBT_FindPath();
		CBT_Occupy* pOccupy = new CBT_Occupy();

		pSequenceNoEnemy->Push(pFindOccupyRegion);
		pSequenceNoEnemy->Push(pNode);
		pSequenceNoEnemy->Push(CreateMoveSelector());
		pSequenceNoEnemy->Push(pOccupy);

        // BT 규칙 검토: 적 존재 분기에 자식이 없어 전투나 대기 행동이 실행되지 않는다.
		CBT_Selector* pSelectorEnemy = new CBT_Selector();
		// 임시노드
		CBT_TempOccpuyCombatNode* pTemp = new CBT_TempOccpuyCombatNode();
		pSelectorEnemy->Push(pTemp);

		pBranchRoot->Push(pSequenceNoEnemy);
		pBranchRoot->Push(pSelectorEnemy);
	}
}


CBT_MoveSelector* CBehaviorManager::CreateMoveSelector()
{
    // Each tree owns a separate movement subtree; raw ownership is never shared.
	CBT_MoveSelector* pMoveSelector = new CBT_MoveSelector();

	CBT_Sequence* pSequenceDoor = new CBT_Sequence();
	CBT_CheckDoor* pCheckDoor = new CBT_CheckDoor();
	CBT_OpenDoor* pOpenDoor = new CBT_OpenDoor();

	pSequenceDoor->Push(pCheckDoor);
	pSequenceDoor->Push(pOpenDoor);

	CBT_Sequence* pSequenceMove = new CBT_Sequence();
	CBT_IsMove* pIsMove = new CBT_IsMove();
	CBT_IsEnemyNearMoveTree* pIsEnemy = new CBT_IsEnemyNearMoveTree();
	CBT_Move* pMove = new CBT_Move();

	pSequenceMove->Push(pIsMove);
	pSequenceMove->Push(pIsEnemy);
	pSequenceMove->Push(pMove);

	{
		pMoveSelector->Push(pSequenceMove);
		pMoveSelector->Push(pSequenceDoor);
	}
    return pMoveSelector;
}

CBT_MoveSelector* CBehaviorManager::CreateCombatMoveSelector()
{
	CBT_MoveSelector* pMoveSelector = new CBT_MoveSelector();

	CBT_Sequence* pSequenceDoor = new CBT_Sequence();
	CBT_CheckDoor* pCheckDoor = new CBT_CheckDoor();
	CBT_OpenDoor* pOpenDoor = new CBT_OpenDoor();

	pSequenceDoor->Push(pCheckDoor);
	pSequenceDoor->Push(pOpenDoor);

	CBT_Sequence* pSequenceMove = new CBT_Sequence();
	CBT_IsMove* pIsMove = new CBT_IsMove();
	CBT_Move* pMove = new CBT_Move();

	pSequenceMove->Push(pIsMove);
	pSequenceMove->Push(pMove);

	{
		pMoveSelector->Push(pSequenceMove);
		pMoveSelector->Push(pSequenceDoor);
	}
	return pMoveSelector;
}
