#pragma once
#include "WinObject.h"

class CBaseObject : public WinObject
{
protected:
    int m_iID = -1;
    int m_iHP = 100;

private:
    int m_iDetectionRadius = 0;
    int m_iRenderDetectionSize = 0;

    int m_iAttackRange = 0;
    int m_iRenderAttackRangeSize = 0;
public:
    int GetId() const { return m_iID; }
    virtual void Init(st_Pos spawnpos) {}
    virtual void OnSpawn() {}
    virtual int OnAttack(int id) { return 0; };
    virtual int OnDamage(int id, int damage) { return 0; };
    virtual void OnDead() {}

    void SetDetectionRadius(int radius)
    {
        m_iDetectionRadius = radius;
        m_iRenderDetectionSize = radius * GetRenderSize();
    }

    void SetAttackRange(int range)
    {
        m_iAttackRange = range;
        m_iRenderAttackRangeSize = range * GetRenderSize();
    }

    int GetDetectionRadius() { return m_iDetectionRadius; }
    int GetRenderDetectionRadiusSize() { return m_iRenderDetectionSize; }
    
    int GetAttackRange() { return m_iAttackRange; }
    int GetRenderAttackRangeSize() { return m_iRenderAttackRangeSize; }
};
