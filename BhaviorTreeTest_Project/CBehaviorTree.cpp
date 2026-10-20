#include "CBehaviorTree.h"
#include "Defines.h"
#include "CSimulatorManager.h"
#include "CEntity.h"
#include "CMap.h"
#include "CDoor.h"
#include "CTeam.h"


int CBT_Selector::Run(st_BlackBoard* bb)
{
    const auto generation = bb->m_runtimeGeneration;
    
    const auto it = bb->m_mapRuntime.find(this);
    int start = it == bb->m_mapRuntime.end() ? 0 : it->second.m_iCurrentIndex;
    
    for (int i = start; i < static_cast<int>(m_vecChild.size()); ++i)
    {
        const int result = m_vecChild[i]->Run(bb);
        
        if (generation != bb->m_runtimeGeneration)
            return eBTresult::Failed;
        
        if (result == eBTresult::Running)
        { 
            bb->PushRuntime(this, {i});
            return result;
        }
        else if (result == eBTresult::Success)
        { 
            ResetSubtree(bb);
            return result;
        }
    }
    
    ResetSubtree(bb);
    return eBTresult::Failed;
}

int CBT_Sequence::Run(st_BlackBoard* bb)
{
    const auto generation = bb->m_runtimeGeneration;
    
    const auto it = bb->m_mapRuntime.find(this);
    int start = m_reactive || it == bb->m_mapRuntime.end() ? 0 : it->second.m_iCurrentIndex;
    for (int i = start; i < static_cast<int>(m_vecChild.size()); ++i)
    {
        const int result = m_vecChild[i]->Run(bb);

        if (generation != bb->m_runtimeGeneration)
            return eBTresult::Failed;
        
        if (result == eBTresult::Running)
        {
            bb->PushRuntime(this, {i});
            return result;
        }
        else if (result != eBTresult::Success) 
        {
            ResetSubtree(bb);
            return eBTresult::Failed;
        }
    }

    ResetSubtree(bb);
    return eBTresult::Success;
}

int CBT_FindNearRegion::Run(st_BlackBoard* bb)
{
    int index = bb->m_pMap->GetNearRegionIndex(bb->m_pEntity->GetPos());
    bb->m_iTargetRegionIndex = index;
    CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iTargetRegionIndex);
	if (pRegion == nullptr)
		return eBTresult::Failed;

    bb->m_stRegionCenterPos = pRegion->GetPos();
    bb->m_eAstarTargetType = eAstarTargetType::Region;

    return eBTresult::Success;
}

int CBT_FindPath::Run(st_BlackBoard* bb)
{
    st_Pos goal;
    switch (bb->m_eAstarTargetType)
    {
	case eAstarTargetType::None:
		return eBTresult::Failed;
	case eAstarTargetType::Region:
        goal = bb->m_stRegionCenterPos;
		break;
	case eAstarTargetType::InRegionRandomPos:
		goal = bb->m_stRegionInsidePos;
        break;
    case eAstarTargetType::Enemy:
        goal = bb->m_stEnemyPos;
        break;
	default:
		return eBTresult::Failed;
    }

    st_Pos start = bb->m_pEntity->GetPos();
    
    bb->m_pEntity->ClearMovePath();
    bool ret = bb->m_pMap->FindPath(start.X, start.Y, goal.X, goal.Y, bb->m_pEntity->GetAstarPath());

    if (!ret)
    {
        bb->m_pEntity->GetAIController()->ReleaseReservation();
        return eBTresult::Failed;
    }

	bb->m_pEntity->InitAstarPathtoMovePath();
	bb->m_stGoalPos = goal;

    return eBTresult::Success;
}

int CBT_Move::Run(st_BlackBoard* bb)
{
    return bb->m_pEntity->Move();
}

CBTNode::~CBTNode()
{
    int nLoop = m_vecChild.size();
    for (int i = 0; i < nLoop; i++)
    {
        delete m_vecChild[i];
    }
}

CBehaviorTree::~CBehaviorTree()
{
    delete m_pRoot;
}

int CBT_CheckDoor::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::Failed;
    st_Pos pos = info.pos;

	CDoor* pDoor = dynamic_cast<CDoor*>(bb->m_pMap->GetPart(pos));

    if(pDoor == nullptr)
		return eBTresult::Failed;

	if (pDoor->GetOpen())
		return eBTresult::Failed;


    return eBTresult::Success;
}

int CBT_OpenDoor::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::Failed;
    st_Pos pos = info.pos;

    CDoor* pDoor = dynamic_cast<CDoor*>(bb->m_pMap->GetPart(pos));
    if (pDoor == nullptr)
        return eBTresult::Failed;

	if (pDoor->GetOpen())
		return eBTresult::Failed;

    pDoor->Use();
    return eBTresult::Success;
}

int CBT_MoveSelector::Run(st_BlackBoard* bb)
{
    int ret = 0;

	st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
    {
		if (bb->m_pEntity->GetPos() == bb->m_stGoalPos)
			return eBTresult::Success;
		return eBTresult::Failed;
    }

    int child;
    // 임시
    switch (info.iType)
    {
	case eSTAT_TYPE::None:
		child = 0;
		break;
	case eSTAT_TYPE::Door:
		child = 1;
		break;
    default:
        child = 0;
        break;
    }
    
    if (child >= static_cast<int>(m_vecChild.size()))
        return eBTresult::Failed;
    
    const auto generation = bb->m_runtimeGeneration;
    
    ret = m_vecChild[child]->Run(bb);
    if (generation != bb->m_runtimeGeneration)
        return eBTresult::Failed;
    if (child == 1 && ret == eBTresult::Success)
        return eBTresult::Running;

    if(ret == eBTresult::Failed && child != 0)
		ret = m_vecChild[0]->Run(bb);

    return ret;
}

int CBT_IsMove::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::Failed;

	CPartsBase* pPart = bb->m_pMap->GetPart(info.pos);

    if (pPart == nullptr)
        return eBTresult::Success;

	return pPart->IsMove() ? eBTresult::Success : eBTresult::Failed;
}

int CBT_ConditionBranch::Run(st_BlackBoard* bb)
{
    if (!m_pCondition)
        return eBTresult::Failed;
    
    const auto generation = bb->m_runtimeGeneration;
    const int branch = m_pCondition->Run(bb);

    if (generation != bb->m_runtimeGeneration)
        return eBTresult::Failed;
    
    if (branch == eBTresult::Running)
        return eBTresult::Running;
    
    if (branch < 0 || branch >= static_cast<int>(m_vecChild.size()))
        return eBTresult::Failed;
    
    for (int i = 0; i < static_cast<int>(m_vecChild.size()); ++i)
    {
        if (i != branch)
            m_vecChild[i]->ResetSubtree(bb);
    }
    
    return m_vecChild[branch]->Run(bb);
}

int CBT_FindEmptyOccupyRegion::Run(st_BlackBoard* bb)
{
    bb->m_pEntity->GetAIController()->ReleaseReservation();
    
    CRegion* best = nullptr;
    long long bestCost = 0;
    
    for (auto* region : bb->m_pMap->GetRegionVector())
    {
        if (region->GetOccupyTeam() == bb->m_pEntity->GetTeam())
            continue;

        const long long cost = bb->m_pEntity->GetPos().LengthLength(region->GetPos())
            + 100LL * bb->m_pTeam->GetReservationCount(region->GetRegionIndex());

        if (!best || cost < bestCost)
        {
            best = region;
            bestCost = cost;
        }
    }

    if (!best)
        return eBTresult::Failed;
    
    bb->m_iTargetRegionIndex = best->GetRegionIndex();
    bb->m_stRegionCenterPos = best->GetPos();
    bb->m_eAstarTargetType = eAstarTargetType::Region;
    bb->m_pEntity->GetAIController()->ReserveRegion(best->GetRegionIndex());
    
    if (bb->m_pMap->GetRegionIndex(bb->m_pEntity->GetPos()) == best->GetRegionIndex())
    {
        bb->m_pEntity->ChangeState(eState::Occupy);
        bb->m_pEntity->GetAIController()->SetOccupyRegionIndex(best->GetRegionIndex());
        return eBTresult::Failed;
    }
    return eBTresult::Success;
}

int CBT_CheckEnemyInRegion::Run(st_BlackBoard* bb)
{
    CRegion* region = bb->m_pMap->GetRegion(bb->m_iOccupyRegionIndex);
    if (!region)
    {
        bb->m_pEntity->ChangeState(eState::Idle);
        return eBTresult::Failed;
    }

    const eTeam enemy = bb->m_pEntity->GetTeam() == eTeam::Red ? eTeam::Blue : eTeam::Red;
    if (region->GetTeamCount(enemy) > 0)
    {
        bb->m_pEntity->ChangeState(eState::Combat);
        return eBTresult::Failed;
    }
    return eBTresult::Failed;
}

int CBT_FindOccupyingRegionInPos::Run(st_BlackBoard* bb)
{
    CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iOccupyRegionIndex);
	if (pRegion == nullptr)
		return eBTresult::Failed;

	bb->m_stRegionInsidePos = pRegion->GetRegionInsidePos();
	bb->m_eAstarTargetType = eAstarTargetType::InRegionRandomPos;

    return eBTresult::Success;
}

int CBT_Occupy::Run(st_BlackBoard* bb)
{
    CRegion* region = bb->m_pMap->GetRegion(bb->m_iOccupyRegionIndex);

    if (!region) 
    {
        bb->m_pEntity->ChangeState(eState::Idle);
        return eBTresult::Failed;
    }

    if (region->GetOccupyTeam() == bb->m_pTeam->GetTeam())
	{
		printf("Occupy Region[%d] eTeam %s\n", bb->m_iOccupyRegionIndex, bb->m_pTeam->GetTeam() == eTeam::Red ? "Red" : "Blue");
		bb->m_pEntity->ChangeState(eState::Idle);

		return eBTresult::Success;
	}

    return eBTresult::Running;
}

void CBehaviorTree::SetRoot(CBTNode* pRoot)
{
    if (m_pRoot != pRoot)
    {
        delete m_pRoot;
        m_pRoot = pRoot;
    }
}

CBT_ConditionBranch::~CBT_ConditionBranch()
{
    delete m_pCondition;
}

void CBT_ConditionBranch::SetCondition(CBTNode* pCondition)
{
    if (m_pCondition != pCondition)
    {
        delete m_pCondition;
        m_pCondition = pCondition;
    }
}

int CBT_IsEnemyNear::Run(st_BlackBoard* bb)
{
    int ret = FindNearEnemys(bb);
    
    if (ret > 0)
    {
        bb->m_iEnemyId = bb->m_vecDetectionEnemy[0]->GetId();
        return eBTresult::Success;
    }

    return eBTresult::Failed;
}

int CBT_IsEnemyNear::FindNearEnemys(st_BlackBoard* bb)
{
    return g_SimulatorManager->FindNearEnemy(bb->m_pEntity, bb->m_vecDetectionEnemy);;
}

int CBT_IsEnemyNearCombatTree::Run(st_BlackBoard* bb)
{
    if (FindNearEnemys(bb))
    {
        return eBTresult::Success;
    }
    bb->m_pEntity->ChangeState(eState::Idle);
    return eBTresult::Failed;
}

int CBT_IsEnemyNearMoveTree::Run(st_BlackBoard* bb)
{
    // MoveTree 에서 적 발견시 Combat 상태로 변경해야하기에 Failed 로 리턴 
    // 이동중에는 발견하지 않는것을 Success 로
    if (FindNearEnemys(bb))
    {
        bb->m_pEntity->ChangeState(eState::Combat);
        return eBTresult::Failed;
    }

    return eBTresult::Success;
}

int CBT_SetEnemyTarget::Run(st_BlackBoard* bb)
{
    g_SimulatorManager->FindNearEnemy(bb->m_pEntity, bb->m_vecDetectionEnemy);
    
    if (bb->m_vecDetectionEnemy.size() <= 0)
        return eBTresult::Failed;

    if (!bb->m_vecDetectionEnemy[0]->GetEnable())
        return eBTresult::Failed;

    bb->m_iEnemyId = bb->m_vecDetectionEnemy[0]->GetId();
    bb->m_stEnemyPos = bb->m_vecDetectionEnemy[0]->GetPos();
    bb->m_eAstarTargetType = eAstarTargetType::Enemy;

    return eBTresult::Success;
}

int CBT_IsAttack::Run(st_BlackBoard* bb)
{
    if (bb->m_iEnemyId < 0)
        return eBTresult::Failed;
    CEntity* pEnemy = g_SimulatorManager->GetEntity(bb->m_iEnemyId);
    if (pEnemy == nullptr)
        return eBTresult::Failed;

    int range = bb->m_pEntity->GetAttackRange();
    int dist = bb->m_pEntity->GetPos().LengthLength(pEnemy->GetPos());
    
    if (range * range < dist)
        return eBTresult::Failed;

    return eBTresult::Success;
}

int CBT_Attack::Run(st_BlackBoard* bb)
{
    return bb->m_pEntity->OnAttack(bb->m_iEnemyId);
}

void CBTNode::ResetSubtree(st_BlackBoard* bb)
{
    bb->m_mapRuntime.erase(this);
    for (auto* child : m_vecChild) child->ResetSubtree(bb);
}
