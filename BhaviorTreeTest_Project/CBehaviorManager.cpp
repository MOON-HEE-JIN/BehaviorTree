#include "CBehaviorManager.h"

CBehaviorManager g_CBehaviorManager;

CBehaviorManager::CBehaviorManager()
{
	InitNodeTrees();
	CreateEntityIdleTree();
}

void CBehaviorManager::CreateEntityIdleTree()
{
	m_IdleTree;

	CBT_Sequence* pSequenceRoot = new CBT_Sequence();

	m_IdleTree.SetRoot(pSequenceRoot);
	{
		CBT_FindEmptyOccupyRegion* pNode = new CBT_FindEmptyOccupyRegion();
		pSequenceRoot->Push(pNode);
	}
	{
		CBT_FindPath* pNode = new CBT_FindPath();
		pSequenceRoot->Push(pNode);
	}
	{
		pSequenceRoot->Push(m_pMoveSelector);
	}
}

void CBehaviorManager::CreateEntityOccupyTree()
{

}

void CBehaviorManager::InitNodeTrees()
{
	CreateMoveSelector();
}

void CBehaviorManager::CreateMoveSelector()
{
	m_pMoveSelector = new CBT_MoveSelector();

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
		m_pMoveSelector->Push(pSequenceMove);
		m_pMoveSelector->Push(pSequenceDoor);
	}
}
