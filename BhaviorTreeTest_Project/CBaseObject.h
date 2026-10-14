#pragma once
#include "WinObject.h"

class CBaseObject : public WinObject
{
protected:
    int m_iID = -1;

private:
    int m_iDetectionRadius = 0;
    int m_iRenderDetectionSize = 0;
    int m_iRenderDectionHalfSize = 0;

public:
    int GetId() const { return m_iID; }
    virtual void Init(st_Pos& spawnpos) {}
    virtual void OnSpawn() {}

    void SetDetectionRadius(int radius)
    {
        m_iDetectionRadius = radius;
        m_iRenderDetectionSize = radius * GetRenderSize();
        m_iRenderDectionHalfSize = static_cast<int>(m_iRenderDetectionSize * 0.5);
    }

    int GetDetectionRadius() { return m_iDetectionRadius; }
    int GetRenderDetectionRadiusSize() { return m_iRenderDetectionSize; }
    int GetRenderDetectionRadiusHalfSize() { return m_iRenderDetectionSize; }
};
