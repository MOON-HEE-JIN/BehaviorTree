#pragma once
#include "CBaseObject.h"

class CItemBase : public CBaseObject
{
public:
	virtual void Initialize() = 0;
	virtual bool Use() = 0;
	virtual bool Destory() = 0;
};