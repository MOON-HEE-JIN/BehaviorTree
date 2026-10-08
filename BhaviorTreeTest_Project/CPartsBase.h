#pragma once
#include "CBaseObject.h"

class CPartsBase : public CBaseObject
{
protected:
	eSTAT_TYPE m_eType = eSTAT_TYPE::None;
	bool m_bMove = false;
public:
	eSTAT_TYPE GetType() const { return m_eType; }

	virtual bool Use() = 0;
	virtual bool Destory() = 0;
	virtual bool IsMove() { return m_bMove; }
};
