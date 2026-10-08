#include "CBehaviorTree.h"
#include "Defines.h"
#include "CEntity.h"
#include "CMap.h"
#include "CDoor.h"
#include "CTeam.h"

int CBT_Selector::Run(st_BlackBoard* bb)
{
    int currentIndex = 0;
    
	if (bb->m_mapRuntime.find(this) != bb->m_mapRuntime.end())
		currentIndex = bb->m_mapRuntime[this].m_iCurrentIndex;
	
    int ret = 0;

    int nLoop = m_vecChild.size();
    for (int i = currentIndex; i < nLoop; i++)
    {
        ret = m_vecChild[i]->Run(bb);

		if (ret == eBTresult::RUNNING)
		{
            bb->PushRuntime(this, st_RumtimeNodeInfo{ ret, i });
			break;
		}
        else 
            break;
    }

    return ret;
}

int CBT_Sequence::Run(st_BlackBoard* bb)
{
    int currentIndex = 0;
    
	if (bb->m_mapRuntime.find(this) != bb->m_mapRuntime.end())
		currentIndex = bb->m_mapRuntime[this].m_iCurrentIndex;

    int ret = 0;

    int nLoop = m_vecChild.size();
    for (int i = currentIndex; i < nLoop; i++)
    {
        ret = m_vecChild[i]->Run(bb);

		if (ret == eBTresult::FAILD)
			break;
        else if (ret == eBTresult::RUNNING)
        {
			bb->PushRuntime(this, st_RumtimeNodeInfo{ ret, i });
            break;
        }
    }

    return ret;
}

int CBT_FindNearRegion::Run(st_BlackBoard* bb)
{
    int index = bb->m_pMap->GetNearRegionIndex(bb->m_pEntity->GetPos());
    bb->m_iTargetRegionIndex = index;
    CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iTargetRegionIndex);
	if (pRegion == nullptr)
		return eBTresult::FAILD;

    bb->m_stRegionCenterPos = pRegion->GetPos();
    bb->m_eAstarTargetType = eAstarTargetType::Region;
    
    return eBTresult::SUCCESS;
}

int CBT_FindPath::Run(st_BlackBoard* bb)
{
    st_Pos goal;
    switch (bb->m_eAstarTargetType)
    {
	case eAstarTargetType::None:
		return eBTresult::FAILD;
	case eAstarTargetType::Region:
        goal = bb->m_stRegionCenterPos;
		break;
	case eAstarTargetType::InRegionRandomPos:
		goal = bb->m_stRegionInsidePos;
        break;
	default:
		return eBTresult::FAILD;
    }

    st_Pos start = bb->m_pEntity->GetPos();

    bool ret = bb->m_pMap->FindPath(start.X, start.Y, goal.X, goal.Y, bb->m_pEntity->GetAstarPath());

	if (ret == false)
		return eBTresult::FAILD;

	bb->m_pEntity->InitAstarPathtoMovePath();
	bb->m_stGoalPos = goal;

    return eBTresult::SUCCESS;
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
    delete m_Root;
}

int CBT_CheckDoor::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::FAILD;
    st_Pos pos = info.pos;

	CDoor* pDoor = dynamic_cast<CDoor*>(bb->m_pMap->GetPart(pos));

    if(pDoor == nullptr)
		return eBTresult::FAILD;

	if (pDoor->GetOpen())
		return eBTresult::FAILD;


    return eBTresult::SUCCESS;
}

int CBT_OpenDoor::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::FAILD;
    st_Pos pos = info.pos;

    CDoor* pDoor = dynamic_cast<CDoor*>(bb->m_pMap->GetPart(pos));
    if (pDoor == nullptr)
        return eBTresult::FAILD;

	if (pDoor->GetOpen())
		return eBTresult::FAILD;

    pDoor->Use();
    return eBTresult::SUCCESS;
}

int CBT_MoveSelector::Run(st_BlackBoard* bb)
{
    int ret = 0;

	st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
    {
		if (bb->m_pEntity->GetPos() == bb->m_stGoalPos)
			return eBTresult::SUCCESS;
		return eBTresult::FAILD;
    }

    // 임시
    switch (info.iType)
    {
	case eSTAT_TYPE::NONE:
		info.iType = 0;
		break;
	case eSTAT_TYPE::DOOR:
		info.iType = 1;
		break;
    }
	ret = m_vecChild[info.iType]->Run(bb);
    
    if(ret == eBTresult::FAILD)
		ret = m_vecChild[0]->Run(bb);

    return ret;
}

int CBT_IsMove::Run(st_BlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::FAILD;

	CPartsBase* pPart = bb->m_pMap->GetPart(info.pos);

    if (pPart == nullptr)
        return eBTresult::SUCCESS;

	return bb->m_pMap->GetPart(info.pos)->IsMove() ? eBTresult::SUCCESS : eBTresult::FAILD;
}

int CBT_ConditionBranch::Run(st_BlackBoard* bb)
{
	int branch = m_pCondition->Run(bb);

	if (branch >= m_vecChild.size())
		return eBTresult::FAILD;

	return m_vecChild[branch]->Run(bb);
}

int CBT_FindEmptyOccupyRegion::Run(st_BlackBoard* bb)
{
	std::vector<int> vecRegionIndex;
    bb->m_pTeam->GetEmptyRegion(vecRegionIndex);

	int index = bb->m_pMap->GetEmptyRegion(vecRegionIndex, bb->m_pEntity->GetPos());

	if (index == -1)
		return eBTresult::FAILD;

    bb->m_iTargetRegionIndex = index;
    
    CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iTargetRegionIndex);
    
    bb->m_stRegionCenterPos = pRegion->GetPos();
    bb->m_eAstarTargetType = eAstarTargetType::Region;

    return eBTresult::SUCCESS;
}

int CBT_CheckEnemyInRegion::Run(st_BlackBoard* bb)
{
	int EnemyCount = bb->m_pMap->GetRegionOppositeCount(bb->m_iOccupyRegionIndex);
    
	if (EnemyCount > 0)
		return eBTresult::SUCCESS;
    return eBTresult::FAILD;
}

int CBT_FindOccupyingRegionInPos::Run(st_BlackBoard* bb)
{
	CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iOccupyRegionIndex);
	if (pRegion == nullptr)
		return eBTresult::FAILD;

	bb->m_stRegionInsidePos = pRegion->GetRegionInsidePos();
	bb->m_eAstarTargetType = eAstarTargetType::InRegionRandomPos;

    return eBTresult::SUCCESS;
}

int CBT_Occupy::Run(st_BlackBoard* bb)
{
	if(bb->m_pMap->GetRegion(bb->m_iOccupyRegionIndex)->GetOccupyTeam() == bb->m_pTeam->GetTeam())
	{
		printf("Occupy Region[%d] eTeam %s\n", bb->m_iOccupyRegionIndex, bb->m_pTeam->GetTeam() == eTeam::RED ? "Red" : "Blue");
		bb->m_pEntity->ChangeState(eState::IDLE);
		bb->m_pTeam->OnOccupyRegion(bb->m_iOccupyRegionIndex, bb->m_pTeam->GetTeam());
		return eBTresult::SUCCESS;
	}
	
    return eBTresult::RUNNING;
}
