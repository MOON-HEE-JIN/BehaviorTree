# 행동트리 기반 지역 점령 시뮬레이션 분석 및 수정 결과

작성일: 2026-10-20 (Asia/Seoul)

현재 작업 사본 `273bc06`을 수정했고, 요청한 참고 브랜치 `feature/tree_adjust_/edit_codex`
(`96ba862`)의 트리·지역 비용·전투 변경을 대조했다. 참고 브랜치를 통째로 병합하거나
다른 작업 사본을 변경하지 않았다. A* 및 Time Wheel 6개 파일은 원본과 동일하다.
기존 구현은 변경 위치의 `// [기존 코드]` 주석으로 보존하고 변경 이유에 `[수정]`을 붙였다.

## A. 프로젝트 구조 분석

### 실행 흐름

`main → StartWinAPIThread → WM_CREATE → CSimulatorManager 생성 → Map/Region/Team/Entity 생성
→ 로직 스레드 → Simulator.Update → Time Wheel 완료 목록 → 객체 Update → 종료 조건 검사`.

별도의 창 스레드는 `WM_PAINT → Simulator.Render → Region/문/Entity 렌더링`을 수행한다.
기존에는 고정 Sleep 후 로직 스레드를 시작했고 두 스레드가 상태를 동기화 없이 공유했다.
이제 생성 완료 후 시작하고 Render/Update를 같은 mutex로 직렬화한다. 창 종료 때 로직을
정지·join한 뒤 Simulator와 GDI+ 자원을 해제한다. GUI 종료 경로의 실행 스트레스 검사는 미실시다.

### 주요 클래스와 소유권

| 클래스 | 책임·소유권 |
|---|---|
| CSimulatorManager | Map, Team, Entity 및 자신의 이벤트 payload 소유; 팀별 Entity 비소유 목록; 최종 결과 확정 |
| CMap | 128×128 셀, Region, 좌표별 문 객체 소유; 하나의 CAstar 인스턴스 사용 |
| CRegion | 단일 점령팀, 현재 점령 진행팀, 실제 등록 Entity 멤버십, 팀별 인원 및 진행 시간 |
| CEntity | CAIEntity/Blackboard, A* 결과 벡터, 이동 큐, HP/상태 및 렌더 자원 |
| CAIEntity | Entity별 Blackboard를 소유하고 Spawn/Idle/Combat/Occupy에 맞는 트리 선택 |
| CBehaviorManager | 프로세스 공용 3개 트리와 독립적으로 소유된 이동 하위 트리 |
| CTeam | 점령 알림 캐시, 지역 ID별 목표 예약 수; 기존 회원 카운터는 승패에 사용하지 않음 |
| CTimerWheelManager | 1ms/1000ms/60000ms 계층 완료 이벤트 전달; 내부 소유권 결함은 별도 보고 |

Entity는 vector에 생성 순서대로 보관되고 ID와 인덱스가 일치한다. 사망은 즉시 삭제가 아니라
Dead+비활성 tombstone이다. 따라서 현재 GetEntity의 O(1) 조회는 유효하다. 향후 중간 erase나
ID 재사용을 도입할 때는 세대 핸들/ID 테이블이 필요하다. 팀별 적 탐지는 여전히 O(E), 탐지 결과
정렬은 O(K log K)다. 공간 인덱스의 효과를 측정하지 않았으므로 이번에 도입하지 않았다.

### 행동트리 정책과 상태 격리

- 일반 Selector/Sequence는 Running 자식 인덱스를 Entity Blackboard에서 기억한다.
- Selector는 첫 성공에서 종료하며 빈 Selector는 실패, 빈 Sequence는 성공이다.
- 이동 안전 조건 Sequence는 reactive로 매 Tick 통행·적 탐지를 검사한다.
- ConditionBranch는 조건을 재평가하고 선택되지 않은 하위 트리의 인덱스를 정리한다.
- 상태 전환은 Blackboard 실행 세대를 증가시킨다. 이전 트리 스택은 이를 확인하고 중단한다.
- 목표 예약, 경로, 타깃과 탐지 목록은 상태 전환 때 해제한다. 트리 노드에는 Entity별 mutable
  실행 상태를 넣지 않았고, reactive 정책만 생성 시 고정한다.

기존 Occupy의 적 분기는 빈 Selector였다. 이제 조건 검사에서 Combat으로 전환하고 현재
트리의 실행을 중단한다. 같은 지역의 적은 탐지 반경 밖이라도 전투 추적 대상으로 포함한다.
전투가 끝나면 Idle에서 지역 소유권을 확인하고, 이미 지역 내부에 있으면 Occupy로 진입한다.
새 전투 스킬이나 스케줄러는 추가하지 않았다.

### 지역 선택·점령·승패

실제 Region 소유권을 기준으로 아군 소유가 아닌 후보를 한 번 순회한다. 비용은
`현재 위치의 제곱거리 + 동일 팀 예약 수 × 100`이다. 예약은 가산 비용이며 후보 제외 조건이
아니다. 같은 비용에서는 Region 생성 순서를 사용한다. 현재 구현의 거리는 경로 비용이 아니다.

참고 브랜치의 비용 갱신마다 vector 탐색+정렬하는 방식 대신 ID별 예약 수를 O(1)에 갱신한다.
각 Entity의 현재 위치가 다르므로 선택할 때 O(R)로 평가한다. 소유팀과 점유 인원, 이동 중 예약을
분리했다. 예약은 목표 변경·경로 실패·전투 진입·사망·종료 때 해제한다.

Region은 중복 입장을 무시하고 실제 등록된 Entity만 퇴장시킨다. 단독 점유 중에만 100ms
논리 Tick만큼 진행하며 양팀 공존 때는 정지한다. 점령 진행팀 변경/빈 지역 전환은 진행량을
초기화하고 기존 소유권은 재점령 완료 때만 바뀐다. 소유권은 eTeam 하나로만 표현된다.

종료는 모든 Region 점령 또는 유효 생존 Entity 소멸이다. 두 조건이 동시에 성립하면
모든 Region 점령을 원인으로 기록한다. 승자는 최종 점령 수만 비교하며 중립은 집계하지 않는다.
양팀 점령 수가 같으면 winner=None(무승부)이다. 첫 Update 전 초기 조건도 검사한다.
기존 무동작 10분 End 이벤트는 보존 주석으로 남겼고 임의의 시간제한 승패 규칙을 추가하지 않았다.

한 완료 배치에서는 기존 등록 순서를 유지하고 매 콜백 직후 종료를 검사한다. 마지막 지역
점령/마지막 사망이 결과를 확정하면 나머지 콜백을 실행하지 않는다. 이는 순차 처리 정책이며
동시 판정이나 팀 공정성을 보장하는 정책은 아니다. 팀 순서에 의한 우선권은 향후 별도 규칙으로
합의할 대상이다. 종료 후 Region Update, Entity 이동/피해/사망 및 신규 이벤트도 차단한다.

## B. 발견된 문제

| 우선순위 | 파일/함수 | 문제점·영향 | 처리 상태 |
|---|---|---|---|
| Critical | CSimulatorManager::Update | 종료 문구만 출력/승패 없음/이벤트 계속 실행 | 종료 원인·결과 고정 및 실행 차단 |
| Critical | WinThread/main | 생성 전 접근 가능, 무한 로직 루프, 렌더/로직 경합 | 생성 후 시작·종료 join·직렬화; GUI 스트레스 미검증 |
| Critical | CEntity::InitAstarPathtoMovePath | 빈 vector.back으로 잘못된 메모리 접근 | 빈 경로 방어 |
| Critical | CAstar::FindPath | 초기 OpenList가 비어도 역참조 | 수정 금지, EditAstar.txt A1 |
| High | CBT_Selector/Sequence | 성공 후 자식 계속 실행, 종료 인덱스 잔존 | 종료 규칙·정리 수정 |
| High | BT/ChangeState | 자식이 ResetRuntime한 뒤 상위 노드가 낡은 상태 재등록 | 실행 세대 검사 |
| High | CEntity::OnDead/GetEntity | 비활성 객체가 전투 조회에 남고 지역 인원 미해제 | Dead 전환·지역/예약 해제·조회 제외 |
| High | CRegion::Enter/Exit/Update | 중복/음수 인원, 빈 지역·경합 중 점령, 30배 시간 증가 | 멤버십·실제 인원·100ms 진행 |
| High | Occupy/Combat | 비어 있는 적 분기, 죽은 타깃 목록, 경계에서 전투 중단 | Combat 전환·탐지 갱신·추적 유지 |
| High | CBT_MoveSelector | 문 열기를 도착 성공으로 전달, 타입 인덱스 범위 초과 가능 | Running 전달·안전한 자식 선택 |
| High | 목표/예약 | 이동 중 점령된 목표와 중단된 예약 잔존 | 실제 소유권 재검사·명시적 해제 |
| High | CMap 입력/FindShape | 누락/잘린 맵을 빈 셀로 처리, 경계 이웃 접근 | 입력 검증·경계 검사 |
| High | Time Wheel | 내부 누수·시간 오차·underflow·취소 계약 | 수정 금지, EditTimewheel.txt |
| Medium | CTeam/목표 선택 | 참고 브랜치의 반복 정렬·비용 중복 누적 가능 | 예약 수와 실제 소유권 분리 |
| Medium | Simulator/Map/문/Region | Entity/Team/parts/GDI 자원 회수 누락 | 소유자 소멸 시 해제; Wheel 내부 누수는 잔존 |
| Medium | 전투 타깃 정렬 | 같은 거리에서 선택 규칙 불명확 | 거리 동률이면 Entity ID |
| Low | SHAPE/기존 유틸 | 부동소수→정수 경고, 미사용 기능/이름 오타 | 기능 변경 없이 제안으로 유지 |

## C. 실제 수정 내역

| 수정 파일·함수 | 변경 이유·내용·동작 변화 | 성능 영향·검증 |
|---|---|---|
| CBehaviorTree.h/.cpp: Selector, Sequence, ConditionBranch, MoveSelector | 성공/실패/Running 규칙, branch 중단, 세대 검사, 문 열기 결과 수정 | 불필요한 자식 실행 차단; 공용 트리 격리/중단 회귀 통과 |
| CBlackBoard.h, CAIEntity.h/.cpp: ResetRuntime, CancelAction, Update | 실행 세대와 Entity별 예약 관리, 중단 시 목표/참조 해제, 목표 점령 재검사 | 예약 누수 제거; 사망/중단/목표 변경 검사 통과 |
| CBehaviorManager.cpp: 이동/추적 Sequence 생성 | Running 동안 적·문 조건을 계속 평가 | 반응성 복원; 경로 요청 횟수의 별도 프로파일은 미실시 |
| CEntity.h/.cpp: Spawn/Dead/ChangeState/Move/경로 초기화 | 초기 점유 등록, 사망 퇴장, 빈 경로 방어, 경로 취소 전에 스텝 소비, 전투 상태 유지 | 범위 오류·빈 큐 pop 방지; ASan 및 전체 시뮬레이션 통과 |
| CRegion.h/.cpp: Enter/Exit/RefreshOccupants/UpdateOccupyTime | 멤버십 중복 제거, 팀별 O(1) 인원, 단독 점유 시간 진행, 소유권 즉시 알림 | 멤버십 메모리 O(현재 점유자); 경합/재점령/퇴장 검사 통과 |
| CTeam.h/.cpp 및 목표 선택 leaf | O(1) 예약 증감, O(R) 실제 후보 선택, 작은 조회 inline | 별도 제한된 벤치마크 수치 참조 |
| CSimulatorManager.h/.cpp: Update/Finish/CreateEntity/조회 | 종료 결과 고정, 이벤트 payload 소유, 중복 등록 차단, 팀 중복 생성 방지, 사망자 조회 제외 | 매 콜백 뒤 O(R+E) 종료 검사; 캐시 최적화는 미도입 |
| CMap.h/.cpp: 입력/FindShape/AddPart/소멸 | 누락·잘린 입력과 경계 방어, 공유 문 중복 생성 회수, Map 소유 parts 해제 | 잘못된 입력은 초기화 예외; 임의 잘못된 맵 전수 검사는 미실시 |
| CDoor, CRegion, CEntity 소멸자 | 소유 GDI 리소스 반환 | 코드 검토/ASan; GDI 핸들 수 장기 측정 미실시 |
| WinThread.cpp/main | 공유 상태 직렬화, 초기화 완료 후 시작, 창 종료 join과 소유자 해제 | GUI 렌더가 로직을 잠시 막을 수 있음; 실행 스트레스 미실시 |
| vcxproj | Windows subsystem 구성에서도 기존 main을 CRT 진입점으로 연결 | 직접 MSVC x64 앱 링크 성공; 모든 VS 구성 빌드는 미실시 |
| tests 및 tools | 실제 클래스를 연결한 회귀·전체 시뮬레이션·재현성 검사·벤치마크 | 재실행 방법은 G 참조 |

기존 코드를 Git 이력만으로 대체하지 않고 변경부 주석에 보존했다. 금지 대상의 원본 인코딩도
변환하지 않았다. 보존된 긴 구간은 의도적으로 실행 코드에서 제외했다.

## D. 최적화 결과

Windows x64, 설치된 MSVC 14.51.36231, 벤치마크만 /O2 /MD /DNDEBUG.
지역 수별 1000회, 고정된 예약 증가 패턴. 참고 브랜치의 scan+sort+min 방식과 실제 CTeam의
인덱스 갱신+선형 최소 조회를 비교했다. 결과 원문은 BenchmarkResults.txt에 있다.

| Region 수 | 참고 평균 ns | 변경 평균 ns | 참고 p95 ns | 변경 p95 ns |
|---|---:|---:|---:|---:|
| 64 | 168.8 | 89.7 | 200 | 100 |
| 256 | 479.0 | 241.3 | 600 | 300 |
| 1024 | 1869.8 | 909.1 | 2000 | 900 |

이는 예약 갱신+조회만의 마이크로벤치마크다. 평균 약 1.9~2.1배 차이는 전체 시뮬레이션의
속도 향상률이 아니다. 거리·소유권 필터·A*·로그·mutex·실제 Entity 분포는 포함하지 않았다.
클럭 분해능과 OS 스케줄링 때문에 특히 최대값/작은 값은 변동이 크다. p99/최대값은 원문 참조.
초기 외부 함수 조회 구현은 큰 배열에서 느린 결과도 나왔고, 작은 조회를 inline한 최종 구현으로
다시 측정했다. O(1) 예약 갱신은 정렬이 없지만 최저 후보 선택은 O(R)다.

현재 Entity별 거리와 소유권 필터가 필요하므로 priority_queue/set/indexed min-heap의
고정 키를 유지하기 어렵다. vector 선형 선택은 구현과 예약 해제 검증이 단순하고 추가 정렬
할당이 없다. 향후 팀 공통 비용·동일 위치 요청이 주가 되면 indexed heap의 O(log R) 갱신과
O(1) 최소 조회를 비교할 수 있다. 이번에는 heap을 도입하지 않았다.

전체 Tick 평균/p95/p99, 경로 요청 수, Tick별 할당 수, 메모리 총량, Entity 수별 서버 처리량은
측정하지 않았다. 종료 검사의 반복 순회와 전투 재탐색은 다음 프로파일 대상이다.

## E. 별도 분석 보고서

- EditAstar.txt: 고립 시작점의 빈 Open Set, 입력 경계, 대각 비용과 휴리스틱 불일치,
  Open/Closed 관리, 정렬·초기화 비용, 실패 출력 계약, 병렬 요청 문제.
- EditTimewheel.txt: Wheel 객체/슬롯 누수, 나머지 시간 처리, Debug 시간 유실,
  unsigned underflow, 등록·취소·중복 계약, 범용 스레드 안전성.

각 항목에 파일/함수, 조건, 영향, 수정안, 우선순위, 연관 시스템 및 검증 방법을 기록했다.
미실행 재현 절차를 테스트 성공으로 표시하지 않았다.

## F. 추가 개선 제안

단기(P0/P1): A* A1/A2와 Time Wheel T1~T5를 별도 승인 범위에서 수정하고 작은 지도
Dijkstra 대조 및 fake clock 타이밍 행렬을 추가한다. 맵 초기화 예외의 부분 생성 자원 회수도
RAII로 보강할 필요가 있다. Windows GUI 종료/리사이즈 스트레스와 CRT leak 검사를 수행한다.

중기(P1/P2): 프로파일로 A* 요청 폭주와 전체 Tick 지연을 먼저 측정한다. 이동 중 적 위치 변화에
대한 경로 재계산 정책·최소 목표 유지 시간·hysteresis를 비교한다. 현재 예약 비용 100은 조정
가능한 설계값이며 정량적인 최적값으로 주장하지 않는다. Region 소유권 변경/생존자 수 캐시를
활용한 종료 검사 최적화, 로그 레벨 제어, 공간 인덱스는 측정 후 결정한다.

장기(P2): 단일 로직 소유 스레드와 명령 큐를 유지하면서 불변 지도/공용 트리 정의와 워커별
탐색/Entity 실행 상태를 분리한다. 변경 명령은 메인 스레드에서 확정한다. 삭제 가능한 Entity는
세대 핸들로 전환하고, 동시 점령의 공정성·타이브레이크를 별도 게임 규칙으로 정의한다.
성능 근거 없이 Entity 병렬 Tick, 락 추가, 전면 ECS 전환은 하지 않았다.

## G. 최종 검증

- 직접 MSVC x64 빌드/링크: GUI 앱과 회귀 테스트 성공.
- 일반 /RTC1 검사 후 AddressSanitizer 빌드로 최종 47개 회귀 검사 통과.
- 양팀 5명 전체 시뮬레이션: 고정 50ms 스텝, Entity 250ms/Region 100ms,
  모든 Region 점령으로 종료, Red 6 / Blue 3, Red 승리.
- 동일 시드 독립 프로세스 2회: 전체 stdout SHA256 일치, 둘 다 exit=0 및 47개 통과.
- ASan: 실행한 입력에서 보고 없음. MSVC ASan은 내부 Wheel 누수 부재를 증명하지 않는다.
- 종료 이후 콜백, 신규 이벤트, 직접 Region Update의 결과 변경 차단 확인.
- 금지된 A*/Time Wheel 6개 파일 원본 대조: 변경 없음.
- git diff --check: 통과.
- MSBuild 솔루션 빌드는 FileTracker E_ACCESSDENIED 때문에 완료하지 못했다.
  시스템 PATH 대소문자 중복도 직접 빌드 도구에서 정규화했다. 저장소 코드를 환경 오류에
  맞추어 변경하지 않고 설치된 컴파일러를 직접 사용했다.
- 미검증: 고립 A* 크래시 실행, 최단성 대조, Time Wheel 전체 시간 경계, CRT/GDI 누수,
  GUI 실제 동시 실행/종료 스트레스, Win32/Release 전체 구성, UBSan/TSan 및 전체 Tick 성능.

테스트 항목 대응: Red/Blue/무승부, 모든 지역 점령, 모든 Entity 사망, 초기 Entity 없음,
초기 점령 완료, 한 팀만 생존, 목표 중복 예약, 이동 목표 점령, 실제 A* false 반환의 호출부
정리, Running 중단, 종료 직전 예약 이벤트를 모두 검사한다. 경로 실패 검사는 A*의
시작==목표 false와 미설정 목표를 사용하며 고립 경로의 크래시 안전성을 보장하지 않는다.

재실행(Visual Studio 18 Community가 설치된 Windows):

```powershell
python tools/build_and_test.py --asan --benchmark
python tools/verify_results.py
```

이 환경에서는 python 실행 별칭 대신 제공된 Python 런타임을 사용했다. 스크립트의 설치 경로는
현재 환경 기준이다. 검증 상세는 TestResults.txt, 원시 로그는 build/asan.log에 있다.
