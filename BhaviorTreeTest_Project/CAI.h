#pragma once

#include "CBehaviorTree.h"

class CAIController
{
public:
	virtual ~CAIController() = default;

protected:
	st_BlackBoard m_blackBoard;
};

