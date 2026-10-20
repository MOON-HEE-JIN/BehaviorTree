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
    ~CRegion() override;
    int GetTeamCount(eTeam team) const;
private:
    std::unordered_map<CEntity*, eTeam> m_members;
    int m_redCount = 0;
    int m_blueCount = 0;
    void RefreshOccupants();
public:
    void AddParts(CPartsBase* pPart);

    // WinObject을(를) 통해 상속됨
    void Update() override;

    void Render(HDC hdc) override;
private:
    st_Rect m_rect;
    HPEN m_hRenderPen;
    HPEN m_hRenderRedPen;
    HPEN m_hRenderBluePen;

    int m_iRenderSize;
    int m_iIndex;
    int m_iProgressValue = 3000;
    float m_fProgress = 0.0f;

    std::vector<CPartsBase*> m_vecRenderParts;
public:
    int GetRegionIndex() const { return m_iIndex; }
    eTeam GetOccupyTeam() const { return m_eOccupyTeam; }

    st_Pos GetRegionInsidePos();
    st_Pos GetRandomRegionDoor();
    st_Pos GetRegionDoor(eDirection eDir);

	int GetOccupyTeamCount() const { return m_iOccupyTeamCount; }
	int GetOppositeCount() const { return m_iOppositeCount; }
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

    eTeam m_eOccupyTeam;                    // 점령팀
    eTeam m_eCurOccupyingTeam;              // 점령중인 팀
    int m_iOccupyTeamCount;                 // 점령 중인 팀 명수
    int m_iOppositeCount;                   // 반대 팀 명수
    int m_iOccupyTimer;                     // 점령에 필요한 시간
    int m_iOccupyingTime;                   // 점령 진행 시간
};

