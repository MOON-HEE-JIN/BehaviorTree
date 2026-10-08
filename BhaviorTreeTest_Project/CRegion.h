#pragma once
#include "WinObject.h"
#include "CBaseObject.h"
#include "UnionDefine.h"
#include <vector>
#include <unordered_map>

class CPartsBase;
class CEntity;

class CRegion :
    public WinObject
{
public:
    CRegion(st_Rect rect, int RenderSize, int index);

    st_Rect m_Rect;

    void AddParts(CPartsBase* pPart);

    // WinObject을(를) 통해 상속됨
    virtual void Update() override;

    virtual void Render(HDC hdc) override;
private:
    HPEN m_hRenderPen;
    HPEN m_hRenderRedPen;
    HPEN m_hRenderBluePen;
    HBRUSH m_hRenderBrushBG;
    HBRUSH m_hRenderBrushCloseDoor;

    int m_iRenderSize;
    int m_iHalfRenderSize;
    int m_iIndex;
    float m_iProgress;

    std::vector<CPartsBase*> m_vecRenderParts;
    std::unordered_map<int, std::vector<CPartsBase*>> m_mapParts;
public:
    int GetRegionIndex() { return m_iIndex; }
    eTeam GetOccupyTeam() { return m_iOccupyTeam; }

    st_Pos GetRegionInsidePos();
    st_Pos GetRandomRegionDoor();
    st_Pos GetRegionDoor(eDirection eDir);

	int GetOccupyTeamCount() { return m_iOccupyTeamCount; }
	int GetOppositeCount() { return m_iOppositeCount; }
public:
    void EnterRegion(eTeam team, CEntity* pObj);           // 지역 들어가기
    void ExitRegion(eTeam team, CEntity* pObj);            // 지역 나가기
    

    void DebugFunc_Test(eTeam occupyTeam, eTeam curoccupyingteam, int occupyTeamCount, int oppositeCount, int occupyTimer, int occupyingTime);
private:
    void UpdateOccupyTime();                // 점령 진행 시간 갱신
    void UpdateInRegion();                  // 안에 있는지 확인

    void EnterNoneRegion(eTeam team);       // 빈 지역
    void EnterOccupyingRegion(eTeam team);  // 점령중인 지역
    void EnterOccupyRegion(eTeam team);     // 점령한 지역
    void OccupyRegion();

    void ChangeOccupyTeam();

    eTeam m_iOccupyTeam;                    // 점령팀
    eTeam m_iCurOccupyingTeam;              // 점령중인 팀
    int m_iOccupyTeamCount;                 // 점령 중인 팀 명수
    int m_iOppositeCount;                   // 반대 팀 명수 
    int m_iOccupyTimer;                     // 점령에 필요한 시간
    int m_iOccupyingTime;                   // 점령 진행 시간
};

