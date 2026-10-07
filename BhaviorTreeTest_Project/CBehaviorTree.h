#pragma once

#include "CBlackBoard.h"
#include <vector>

class CBTNode
{
public:
	~CBTNode();
	virtual int Run(CBlackBoard* bb) = 0;
protected:
	std::vector<CBTNode*> m_vecChild;

public:
	void Push(CBTNode* pNode) { m_vecChild.push_back(pNode); }
};

class CBehaviorTree
{
public:
	~CBehaviorTree();
	void Run(CBlackBoard* bb) { m_Root->Run(bb); };
private:
	CBTNode* m_Root;
public:
	void SetRoot(CBTNode* pRoot) { m_Root = pRoot; }
};

class CBT_Selector : public CBTNode
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_Sequence : public CBTNode
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_ConditionBranch : public CBTNode
{
private:
	CBTNode* m_pCondition;

public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_Condition : public CBTNode
{
public:
	virtual int Run(CBlackBoard* bb) {return 0;};
};

class CBT_Leaf : public CBTNode
{
public:
	virtual int Run(CBlackBoard* bb) { return 0; };
};

// IDLE 상태일때
// 1. 가까운 위치 찾기
// 2. 이동 거리 추적
// 3. 이동
//

class CBT_FindNearRegion : public CBT_Condition
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_FindPath : public CBT_Condition
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_MoveSelector : public CBT_Selector
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_CheckDoor : public CBT_Condition
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_OpenDoor : public CBT_Leaf
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_IsMove : public CBT_Condition
{
public:
	virtual int Run(CBlackBoard* bb);
};

class CBT_Move : public CBT_Leaf
{
public:
	virtual int Run(CBlackBoard* bb);
};

// OCCUPY 상태일때
// 1. 점령중인 지역 내 적군 존재 여부 확인
// 1-1. 적군 존재시 점령중인 지역 내 적군과의 전투
// 1-2. 점령중인 지역 내 적군 존재하지 않을시 점령중인 지역 내 점령 진행
//

class CBT_CheckEnemyInRegion : public CBT_Condition
{
public:
	virtual int Run(CBlackBoard* bb);
};

