#include "CBehaviorManager.h"

CBehaviorManager g_CBehaviorManager;

CBehaviorManager::CBehaviorManager()
{
	CreateEntityIdleTree();
	CreateEntityOccupyTree();
}

void CBehaviorManager::CreateEntityIdleTree()
{

	CBT_Sequence* pSequenceRoot = new CBT_Sequence();

	m_idleTree.SetRoot(pSequenceRoot);
	{
		CBT_FindEmptyOccupyRegion* pNode = new CBT_FindEmptyOccupyRegion();
		pSequenceRoot->Push(pNode);
	}
	{
		CBT_FindPath* pNode = new CBT_FindPath();
		pSequenceRoot->Push(pNode);
	}
	{
		pSequenceRoot->Push(CreateMoveSelector());
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
	CBT_Move* pMove = new CBT_Move();

	pSequenceMove->Push(pIsMove);
	pSequenceMove->Push(pMove);

	{
		pMoveSelector->Push(pSequenceMove);
		pMoveSelector->Push(pSequenceDoor);
	}
    return pMoveSelector;
}
