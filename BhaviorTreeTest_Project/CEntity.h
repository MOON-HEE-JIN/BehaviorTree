#pragma once
#include "CBaseObject.h"
#include "CAIEntity.h"
#include <vector>
#include <queue>
#include <map>
#include <gdiplus.h>

class CTeam;

class CEntity :
    public CBaseObject
{
public:
    CEntity(eTeam team, int id);
    ~CEntity() override = default;

private:
    eTeam m_eTeam;
    eState m_eState = eState::None;
    int m_iCurrentRegionIndex = -1;
    CAIEntity m_ai;

    std::vector<st_Pos> m_vecAstarPath;     // Astar 이동 경로 노드
    std::queue<st_PosInfo> m_queueMovePath;      // 노드 간의 이동 경로

    Gdiplus::SolidBrush m_hDetectionBrush;
    Gdiplus::Pen m_hDetectionPen;
private:
    void RenderCharacter(HDC hdc);
    void RenderDetection(HDC hdc);
public:
    // CBaseObject을(를) 통해 상속됨
    void Init(st_Pos& spawnpos) override;
    void Update() override;
    void Render(HDC hdc) override;
    void Render_Layer_01(HDC hdc) override;
    void OnSpawn() override;
    int OnAttack(int id) override;
    int OnDamage(int id, int damage) override;
    void OnDead() override;

    eTeam GetTeam() const { return m_eTeam; }
    eState GetState() const { return m_eState; }
    CAIEntity* GetAIController() { return &m_ai; }
    std::vector<st_Pos>& GetAstarPath() { return m_vecAstarPath; }
    bool GetMoveNextPosInfo(st_PosInfo& posinfo);

    void SetTeam(CTeam* pTeam) { m_ai.SetTeam(pTeam); }

    void InitAstarPathtoMovePath();
    void ChangeState(eState state);

    bool NetMovePath();
    int Move();
};

