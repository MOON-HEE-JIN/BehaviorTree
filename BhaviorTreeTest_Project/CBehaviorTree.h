#pragma once

#include "CBlackBoard.h"
#include <vector>

class CBTNode
{
public:
	CBTNode() = default;
    virtual ~CBTNode();
    CBTNode(const CBTNode&) = delete;
    CBTNode& operator=(const CBTNode&) = delete;
	virtual int Run(st_BlackBoard* bb) = 0;
protected:
	std::vector<CBTNode*> m_vecChild;

public:
	void Push(CBTNode* pNode) { m_vecChild.push_back(pNode); }
};

class CBehaviorTree
{
public:
	CBehaviorTree() = default;
    ~CBehaviorTree();
    CBehaviorTree(const CBehaviorTree&) = delete;
    CBehaviorTree& operator=(const CBehaviorTree&) = delete;
	void Run(st_BlackBoard* bb) { if (m_pRoot != nullptr) m_pRoot->Run(bb); }
private:
	CBTNode* m_pRoot = nullptr;
public:
	void SetRoot(CBTNode* pRoot);
};

class CBT_Selector : public CBTNode
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_Sequence : public CBTNode
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_ConditionBranch : public CBTNode
{
private:
	CBTNode* m_pCondition = nullptr;

public:
    ~CBT_ConditionBranch() override;
    void SetCondition(CBTNode* pCondition);
	int Run(st_BlackBoard* bb) override;
};

class CBT_Condition : public CBTNode
{
public:
	int Run(st_BlackBoard* bb) override {return 0;};
};

class CBT_Leaf : public CBTNode
{
public:
	int Run(st_BlackBoard* bb) override { return 0; };
};

// Idle 상태일때
// 1. 가까운 위치 찾기
// 2. 이동 거리 추적
// 3. 이동
//

class CBT_FindNearRegion : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_FindEmptyOccupyRegion : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_FindPath : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_MoveSelector : public CBT_Selector
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_CheckDoor : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_OpenDoor : public CBT_Leaf
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_IsMove : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_Move : public CBT_Leaf
{
public:
	int Run(st_BlackBoard* bb) override;
};

// Occupy 상태일때
// 1. 점령중인 지역 내 적군 존재 여부 확인
// 1-1. 적군 존재시 점령중인 지역 내 적군과의 전투
// 1-2. 점령중인 지역 내 적군 존재하지 않을시 점령중인 지역 내 점령 진행
// 2. 점령중인 지역 내 위치로 이동
// 3. 점령중인 지역 내 위치에서 점령 진행
//

class CBT_CheckEnemyInRegion : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_FindOccupyingRegionInPos : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_Occupy : public CBT_Leaf
{
public:
	int Run(st_BlackBoard* bb) override;
};

class CBT_IsEnemyNear : public CBT_Condition
{
public:
	int Run(st_BlackBoard* bb) override;
};