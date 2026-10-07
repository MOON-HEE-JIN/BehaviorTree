#pragma once
#include "CBaseObject.h"

class CPartsBase : public CBaseObject
{
protected:
	eSTAT_TYPE type;
	bool m_bMove;
public:
	eSTAT_TYPE GetType() { return type; }

	virtual bool Use() = 0;
	virtual bool Destory() = 0;
	virtual bool IsMove() { return m_bMove; }
};