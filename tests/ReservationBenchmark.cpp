// [수정] 참고 브랜치의 예약 갱신+최소 조회와 직접 인덱스 갱신+선형 조회를 비교한다.
// 전체 서버 Tick 벤치마크가 아니며 거리/소유권 필터 비용은 제외한다.
#include "CTeam.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

struct Cost { int index; int cost; };
static volatile int checksum = 0;

static void Print(int regions, const char* name, std::vector<double> ns)
{
    double average = std::accumulate(ns.begin(), ns.end(), 0.0) / ns.size();
    std::sort(ns.begin(), ns.end());
    std::cout << regions << ',' << name << ',' << average << ',' << ns[ns.size() * 95 / 100]
        << ',' << ns[ns.size() * 99 / 100] << ',' << ns.back() << '\n';
}

int main()
{
    using Clock = std::chrono::steady_clock;
    std::cout << "regions,operation,average_ns,p95_ns,p99_ns,max_ns\n";
    for (int count : {64, 256, 1024})
    {
        std::vector<Cost> reference;
        for (int i = 0; i < count; ++i) reference.push_back({i, 0});
        CTeam team(eTeam::Red, count);
        std::vector<double> before, after;
        for (int sample = 0; sample < 1000; ++sample)
        {
            const int index = sample % count;
            auto start = Clock::now();
            for (auto& cost : reference) if (cost.index == index) cost.cost += 10;
            std::sort(reference.begin(), reference.end(), [](const Cost& a, const Cost& b) { return a.cost < b.cost; });
            checksum = reference.front().index;
            before.push_back(std::chrono::duration<double, std::nano>(Clock::now() - start).count());
            start = Clock::now();
            team.ChangeReservation(index, 1);
            int best = 0;
            int bestCost = team.GetReservationCount(0);
            for (int i = 1; i < count; ++i)
            {
                const int cost = team.GetReservationCount(i);
                if (cost < bestCost) { best = i; bestCost = cost; }
            }
            checksum = best;
            after.push_back(std::chrono::duration<double, std::nano>(Clock::now() - start).count());
        }
        Print(count, "reference_scan_sort_min", before);
        Print(count, "indexed_update_linear_min", after);
    }
}
