// [수정] 실제 프로젝트 클래스를 연결하여 상태 전환, 점령, 종료 불변식을 재현한다.
#include "CSimulatorManager.h"
#include "CBehaviorTree.h"
#include <gdiplus.h>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <chrono>

static int checks = 0;
static void Check(bool value, const char* message)
{
    ++checks;
    if (!value) throw std::runtime_error(message);
}

class ScriptNode : public CBTNode
{
public:
    std::function<int(st_BlackBoard*)> script;
    explicit ScriptNode(std::function<int(st_BlackBoard*)> f) : script(f) {}
    int Run(st_BlackBoard* bb) override { return script(bb); }
};

class CounterObject : public WinObject
{
public:
    int calls = 0;
    CEntity* victim = nullptr;
    void Update() override { ++calls; if (victim) victim->OnDead(); }
    void Render(HDC) override {}
};

static TWHEEL* Event(WinObject* object, int delay = 1)
{
    auto* event = new TWHEEL{};
    event->TotalTimer = delay;
    event->pObj = object;
    event->eUpdateType = eTUpdateType::Object;
    return event;
}

struct Simulation
{
    CSimulatorManager manager;
    Simulation(int red = 0, int blue = 0) : manager(red, blue) { g_SimulatorManager = &manager; }
    ~Simulation() { g_SimulatorManager = nullptr; }
};

static void CompositeTests()
{
    st_BlackBoard a, b;
    int first = 0, second = 0;
    CBT_Selector selector;
    selector.Push(new ScriptNode([&](st_BlackBoard*) { ++first; return eBTresult::Success; }));
    selector.Push(new ScriptNode([&](st_BlackBoard*) { ++second; return eBTresult::Success; }));
    Check(selector.Run(&a) == eBTresult::Success && first == 1 && second == 0, "selector must stop at success");
    CBT_Selector emptySelector;
    CBT_Sequence emptySequence;
    Check(emptySelector.Run(&a) == eBTresult::Failed, "empty selector");
    Check(emptySequence.Run(&a) == eBTresult::Success, "empty sequence");
    int conditionCalls = 0;
    bool running = true;
    CBT_Sequence sequence;
    sequence.Push(new ScriptNode([&](st_BlackBoard*) { ++conditionCalls; return eBTresult::Success; }));
    sequence.Push(new ScriptNode([&](st_BlackBoard*) { return running ? eBTresult::Running : eBTresult::Success; }));
    Check(sequence.Run(&a) == eBTresult::Running, "sequence running");
    Check(sequence.Run(&a) == eBTresult::Running && conditionCalls == 1, "memory resumes running child");
    Check(sequence.Run(&b) == eBTresult::Running && conditionCalls == 2, "shared tree isolates entity runtime");
    running = false;
    Check(sequence.Run(&a) == eBTresult::Success && a.m_mapRuntime.empty(), "terminal success clears runtime");
    sequence.Run(&a);
    Check(conditionCalls == 3, "sequence restarts at first child");
    CBT_Sequence interrupt;
    int after = 0;
    interrupt.Push(new ScriptNode([](st_BlackBoard* bb) { bb->ResetRuntime(); return eBTresult::Running; }));
    interrupt.Push(new ScriptNode([&](st_BlackBoard*) { ++after; return eBTresult::Success; }));
    Check(interrupt.Run(&a) == eBTresult::Failed && a.m_mapRuntime.empty() && after == 0, "state change must not recreate runtime");
    bool allowed = true;
    CBT_Sequence reactive(true);
    reactive.Push(new ScriptNode([&](st_BlackBoard*) { return allowed ? eBTresult::Success : eBTresult::Failed; }));
    reactive.Push(new ScriptNode([](st_BlackBoard*) { return eBTresult::Running; }));
    reactive.Run(&a);
    allowed = false;
    Check(reactive.Run(&a) == eBTresult::Failed && a.m_mapRuntime.empty(), "reactive guard aborts running movement");
    CBT_ConditionBranch branch;
    Check(branch.Run(&a) == eBTresult::Failed, "missing condition safe");
    int chosen = eBTresult::Failed;
    auto* child = new CBT_Sequence;
    child->Push(new ScriptNode([](st_BlackBoard*) { return eBTresult::Running; }));
    branch.SetCondition(new ScriptNode([&](st_BlackBoard*) { return chosen; }));
    branch.Push(child);
    branch.Push(new ScriptNode([](st_BlackBoard*) { return eBTresult::Success; }));
    branch.Run(&a);
    chosen = eBTresult::Success;
    Check(branch.Run(&a) == eBTresult::Success && a.m_mapRuntime.empty(), "branch change clears interrupted subtree");
    running = true;
    sequence.Run(&a);
    running = false;
    // Force the remembered leaf to fail and verify restart after failure.
    CBT_Sequence failing;
    bool fail = false;
    failing.Push(new ScriptNode([&](st_BlackBoard*) { return fail ? eBTresult::Failed : eBTresult::Running; }));
    failing.Run(&a); fail = true;
    Check(failing.Run(&a) == eBTresult::Failed && !a.m_mapRuntime.count(&failing), "failure clears runtime");
}

static void RegionTests()
{
    CRegion region(st_Rect(1, 1, 5, 5), 8, 0);
    CEntity red(eTeam::Red, 100), blue(eTeam::Blue, 101);
    region.EnterRegion(eTeam::Red, &red);
    region.EnterRegion(eTeam::Red, &red);
    Check(region.GetTeamCount(eTeam::Red) == 1, "duplicate enter");
    region.DebugFunc_Test(eTeam::None, eTeam::Red, 1, 0, 200, 0);
    region.Update();
    Check(region.GetOccupyTeam() == eTeam::None, "capture progresses by tick rather than 3000ms");
    region.EnterRegion(eTeam::Blue, &blue);
    for (int i = 0; i < 5; ++i) region.Update();
    Check(region.GetOccupyTeam() == eTeam::None, "contested region pauses");
    region.ExitRegion(eTeam::Blue, &blue);
    region.Update();
    Check(region.GetOccupyTeam() == eTeam::Red, "red captures after enemy leaves");
    region.ExitRegion(eTeam::Red, &red);
    region.ExitRegion(eTeam::Red, &red);
    Check(region.GetTeamCount(eTeam::Red) == 0 && region.GetOppositeCount() == 0, "duplicate exit safe");
    region.EnterRegion(eTeam::Blue, &blue);
    region.Update(); region.Update();
    Check(region.GetOccupyTeam() == eTeam::Blue, "blue recaptures without corrupting team counts");
    region.ExitRegion(eTeam::Blue, &blue);
    region.DebugFunc_Test(eTeam::None, eTeam::None, 0, 0, 100, 0);
    region.Update();
    Check(region.GetOccupyTeam() == eTeam::None, "empty region cannot capture");
}

static void ResultCase(int red, int blue, eTeam winner, bool captured)
{
    Simulation simulation;
    auto* map = simulation.manager.GetMap();
    const auto& regions = map->GetRegionVector();
    Check(static_cast<int>(regions.size()) >= red + blue, "fixture region count");
    for (int i = 0; i < static_cast<int>(regions.size()); ++i)
        regions[i]->DebugFunc_Test(i < red ? eTeam::Red : i < red + blue ? eTeam::Blue : eTeam::None, eTeam::None, 0, 0, 100, 0);
    simulation.manager.Update();
    const auto& result = simulation.manager.GetResult();
    Check(result.red == red && result.blue == blue && result.winner == winner, "winner uses captured regions");
    Check(result.reason == (captured ? CSimulatorManager::EndReason::AllRegionsCaptured : CSimulatorManager::EndReason::NoLivingEntities), "end reason separate from winner");
}

static void SimulationTests()
{
    ResultCase(2, 1, eTeam::Red, false);
    ResultCase(1, 2, eTeam::Blue, false);
    ResultCase(1, 1, eTeam::None, false);
    ResultCase(0, 0, eTeam::None, false);
    {
        Simulation sim;
        int count = sim.manager.GetMap()->GetRegionCount();
        for (auto* region : sim.manager.GetMap()->GetRegionVector())
            region->DebugFunc_Test(eTeam::Red, eTeam::None, 0, 0, 100, 0);
        sim.manager.Update();
        Check(sim.manager.GetResult().red == count && sim.manager.GetResult().reason == CSimulatorManager::EndReason::AllRegionsCaptured, "initial all-captured state");
    }
    {
        Simulation sim(2, 0);
        Check(!sim.manager.IsFinished(), "one surviving team does not end early");
        auto* first = sim.manager.GetEntity(0);
        auto* second = sim.manager.GetEntity(1);
        first->OnSpawn(); second->OnSpawn();
        auto* team = sim.manager.GetTeam(eTeam::Red);
        first->GetAIController()->ReserveRegion(0);
        second->GetAIController()->ReserveRegion(0);
        Check(team->GetReservationCount(0) == 2, "multiple entities reserve same region");
        first->ChangeState(eState::Combat);
        Check(team->GetReservationCount(0) == 1, "interruption releases only own reservation");
        first->GetAIController()->ReserveRegion(0);
        const int index = first->GetCurrentRegionIndex();
        first->OnDead(); first->OnDead();
        Check(!sim.manager.GetEntity(0) && first->GetState() == eState::Dead && team->GetReservationCount(0) == 1, "death releases reservation once");
        if (auto* region = sim.manager.GetMap()->GetRegion(index)) Check(region->GetTeamCount(eTeam::Red) == 1, "death exits region");
        second->OnDead();
        sim.manager.Update();
        Check(sim.manager.GetResult().reason == CSimulatorManager::EndReason::NoLivingEntities && team->GetReservationCount(0) == 0, "all deaths finish simulation");
    }
    {
        Simulation sim(1, 0);
        CounterObject killer, late;
        killer.victim = sim.manager.GetEntity(0);
        killer.SetUpdateTime(0);
        auto* once = Event(&killer);
        sim.manager.NewUpdatePushTimerWheel(once);
        sim.manager.NewUpdatePushTimerWheel(once);
        sim.manager.NewUpdatePushTimerWheel(Event(&late));
        Sleep(5);
        for (int i = 0; i < 50 && !sim.manager.IsFinished(); ++i) { Sleep(1); sim.manager.Update(); }
        Check(killer.calls == 1 && late.calls == 0 && sim.manager.IsFinished(), "pending same-batch event suppressed after last death");
        sim.manager.NewUpdatePushTimerWheel(Event(&late));
        sim.manager.Update();
        Check(late.calls == 0, "finished simulation rejects new events");
    }
    {
        Simulation sim(2, 0);
        auto* map = sim.manager.GetMap();
        auto* entity = sim.manager.GetEntity(0);
        entity->OnSpawn();
        st_BlackBoard bb;
        bb.m_pEntity = entity; bb.m_pMap = map; bb.m_pTeam = sim.manager.GetTeam(eTeam::Red);
        CBT_FindEmptyOccupyRegion choose;
        choose.Run(&bb);
        const auto& regions = map->GetRegionVector();
        Check(!regions.empty(), "map regions discovered");
        // ReserveRegion normally belongs to the entity Blackboard; the leaf updates this external test Blackboard too.
        int selected = bb.m_iTargetRegionIndex;
        if (selected >= 0)
        {
            regions[selected]->DebugFunc_Test(eTeam::Red, eTeam::None, 0, 0, 100, 0);
            entity->Update();
            Check(bb.m_pTeam->GetReservationCount(selected) == 0, "captured target releases reservation during movement");
        }
        entity->InitAstarPathtoMovePath();
        Check(map->GetRegionIndex(st_Pos(-1, 0)) == -1 && map->GetRegion(-1) == nullptr, "invalid indices safe");
        // A* regression: safe non-isolated disconnected map, empty initial open set is tested in a subprocess separately.
        st_BlackBoard pathbb;
        pathbb.m_pEntity = entity; pathbb.m_pMap = map;
        CBT_FindPath find;
        Check(find.Run(&pathbb) == eBTresult::Failed, "unset path target fails safely");
        pathbb.m_eAstarTargetType = eAstarTargetType::Region;
        pathbb.m_stRegionCenterPos = entity->GetPos();
        entity->GetAIController()->ReserveRegion(0);
        entity->GetAstarPath().push_back(st_Pos(1, 1));
        Check(find.Run(&pathbb) == eBTresult::Failed && entity->GetAstarPath().empty()
            && sim.manager.GetTeam(eTeam::Red)->GetReservationCount(0) == 0,
            "Astar same-position failure clears stale path and reservation");
    }
}

// [수정] 벽시계/Time Wheel의 시간 오차와 분리해 실제 행동 노드와 Region을 고정 간격으로 실행한다.
static CSimulatorManager::Result FullSimulation()
{
    Simulation sim(5, 5);
    for (int step = 0; step < 100000 && !sim.manager.IsFinished(); ++step)
    {
        if (step % 5 == 0)
            for (int id = 0; id < 10 && !sim.manager.IsFinished(); ++id)
            {
                if (auto* entity = sim.manager.GetEntity(id)) entity->Update();
                sim.manager.CheckEndCondition();
            }
        if (step % 2 == 0)
            for (auto* region : sim.manager.GetMap()->GetRegionVector())
            {
                if (sim.manager.IsFinished()) break;
                region->Update();
                sim.manager.CheckEndCondition();
            }
    }
    Check(sim.manager.IsFinished(), "full 5v5 simulation reaches a defined end condition");
    const auto result = sim.manager.GetResult();
    const auto original = result;
    for (auto* region : sim.manager.GetMap()->GetRegionVector()) region->Update();
    sim.manager.Update();
    Check(sim.manager.GetResult().red == original.red && sim.manager.GetResult().blue == original.blue, "result stays frozen after end");
    std::cout << "FULL SIMULATION: reason=" << static_cast<int>(result.reason) << " red=" << result.red << " blue=" << result.blue << '\n';
    return result;
}

int main()
{
    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &input, nullptr);
    try
    {
        CompositeTests();
        RegionTests();
        SimulationTests();
        FullSimulation();
        std::cout << "PASS: " << checks << " regression checks\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL after " << checks << ": " << error.what() << '\n';
        return 1;
    }
    Gdiplus::GdiplusShutdown(token);
    return 0;
}
