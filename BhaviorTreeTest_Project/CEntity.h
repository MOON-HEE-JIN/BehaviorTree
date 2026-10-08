#pragma once
#include "CBaseObject.h"
#include "CAIEntity.h"
#include <vector>
#include <queue>
#include <map>

class CTeam;

class CEntity :
    public CBaseObject
{
public:
    CEntity(eTeam team, int id);
    ~CEntity();

private:
    eTeam m_eTeam;
    eState m_eState;
    CAIEntity m_Ai;

    std::vector<st_Pos> m_vecAstarPath;     // Astar 이동 경로 노드
    std::queue<st_PosInfo> m_queueMovePath;      // 노드 간의 이동 경로
public:
    // CBaseObject을(를) 통해 상속됨
    void Init(st_Pos& spawnpos) override;
    void Update() override;
    void Render(HDC hdc) override;
    void OnSpawn() override;

    eTeam GetTeam() { return m_eTeam; }
    eState GetState() { return m_eState; }
    CAIEntity* GetAIContorller() { return &m_Ai; }
    std::vector<st_Pos>& GetAstarPath() { return m_vecAstarPath; }
    bool GetMoveNextPosInfo(st_PosInfo& posinfo);

    void SetTeam(CTeam* pTeam) { m_Ai.SetTeam(pTeam); }

    void InitAstarPathtoMovePath();
    void ChangeState(eState state);

    bool NetMovePath();
    int Move();
};

