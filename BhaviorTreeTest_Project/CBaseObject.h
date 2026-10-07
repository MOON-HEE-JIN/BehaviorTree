#pragma once
#include "WinObject.h"

class CBaseObject :
    public WinObject
{
public:
    int m_iID;
    eTeam m_eTeam;
    int m_iHP = 100;
    int m_iCurrentRegionIndex;

    virtual void Init(st_Pos& spawnpos) {};
    virtual void OnSpawn() {};
};

