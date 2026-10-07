#include "CBehaviorTree.h"
#include "Defines.h"
#include "CEntity.h"
#include "CMap.h"
#include "CDoor.h"
#include "CTeam.h"

int CBT_Selector::Run(CBlackBoard* bb)
{
    int currentIndex = 0;
    if (bb->m_stRuntime.m_iRet == eBTresult::RUNNING)
        currentIndex = bb->m_stRuntime.m_iCurrentIndex;

    int ret = 0;

    int nLoop = m_vecChild.size();
    for (int i = 0; i < nLoop; i++)
    {
        ret = m_vecChild[i]->Run(bb);

        switch (ret)
        {
        case eBTresult::FAILD:
            break;
        case eBTresult::SUCCESS:
            break;
        case eBTresult::RUNNING:
            bb->m_stRuntime.m_iRet = ret;
            bb->m_stRuntime.m_iCurrentIndex = i;
            break;
        default:
            break;
        }
    }

    return ret;
}

int CBT_Sequence::Run(CBlackBoard* bb)
{
    int currentIndex = 0;
    if (bb->m_stRuntime.m_iRet == eBTresult::RUNNING)
        currentIndex = bb->m_stRuntime.m_iCurrentIndex;

    int ret = 0;

    int nLoop = m_vecChild.size();
    for (int i = 0; i < nLoop; i++)
    {
        ret = m_vecChild[i]->Run(bb);

        switch (ret)
        {
        case eBTresult::FAILD:
            break;
        case eBTresult::RUNNING:
            bb->m_stRuntime.m_iRet = ret;
            bb->m_stRuntime.m_iCurrentIndex = i;
            break;
        default:
            break;
        }
    }

    return ret;
}

int CBT_FindNearRegion::Run(CBlackBoard* bb)
{
    int index = bb->m_pMap->GetNearRegionIndex(bb->m_pEntity->GetPos());
    bb->m_iTargetRegionIndex = index;
    bb->m_eAstarTargetType = eAstarTargetType::Region;
    return eBTresult::SUCCESS;
}

int CBT_FindPath::Run(CBlackBoard* bb)
{
    st_Pos goal;
    switch (bb->m_eAstarTargetType)
    {
	case eAstarTargetType::None:
		return eBTresult::FAILD;
	case eAstarTargetType::Region:
    {
        CRegion* pRegion = bb->m_pMap->GetRegion(bb->m_iTargetRegionIndex);
        goal = pRegion->GetPos();
    }
		break;
	case eAstarTargetType::Pos:
		return eBTresult::FAILD;
	default:
		return eBTresult::FAILD;
    }

    st_Pos start = bb->m_pEntity->GetPos();

    bool ret = bb->m_pMap->FindPath(start.X, start.Y, goal.X, goal.Y, bb->m_pEntity->GetAstarPath());

	if (ret == false)
		return eBTresult::FAILD;

	bb->m_pEntity->InitAstarPathtoMovePath();

    return eBTresult::SUCCESS;
}

int CBT_Move::Run(CBlackBoard* bb)
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

int CBT_CheckDoor::Run(CBlackBoard* bb)
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

int CBT_OpenDoor::Run(CBlackBoard* bb)
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

int CBT_MoveSelector::Run(CBlackBoard* bb)
{
    int ret = 0;

	st_PosInfo info;
	if(!bb->m_pEntity->GetMoveNextPosInfo(info))
		return eBTresult::FAILD;

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

int CBT_IsMove::Run(CBlackBoard* bb)
{
    st_PosInfo info;
    if (!bb->m_pEntity->GetMoveNextPosInfo(info))
        return eBTresult::FAILD;

	CPartsBase* pPart = bb->m_pMap->GetPart(info.pos);

    if (pPart == nullptr)
        return eBTresult::SUCCESS;

	return bb->m_pMap->GetPart(info.pos)->IsMove() ? eBTresult::SUCCESS : eBTresult::FAILD;
}

int CBT_ConditionBranch::Run(CBlackBoard* bb)
{
	int branch = m_pCondition->Run(bb);

	if (branch >= m_vecChild.size())
		return eBTresult::FAILD;

	return m_vecChild[branch]->Run(bb);
}

int CBT_FindEmptyOccupyRegion::Run(CBlackBoard* bb)
{
	std::vector<int> vecRegionIndex;
    bb->m_pTeam->GetEmptyRegion(vecRegionIndex);

	int index = bb->m_pMap->GetEmptyRegion(vecRegionIndex, bb->m_pEntity->GetPos());

	if (index == -1)
		return eBTresult::FAILD;

    bb->m_iTargetRegionIndex = index;
    bb->m_eAstarTargetType = eAstarTargetType::Region;

    return eBTresult::SUCCESS;
}
