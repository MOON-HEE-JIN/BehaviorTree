#pragma once
#include "WinObject.h"

class CBaseObject : public WinObject
{
protected:
    int m_iID = -1;

public:
    int GetId() const { return m_iID; }
    virtual void Init(st_Pos& spawnpos) {}
    virtual void OnSpawn() {}
};
