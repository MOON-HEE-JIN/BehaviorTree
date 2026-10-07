#pragma once
#include <vector>
#include <list>
#include <Windows.h>
#include "UnionDefine.h"

class CTimerWheel
{
public:
	CTimerWheel(int tick, int wheelsize);
	~CTimerWheel();

private:
	int m_iTick;
	int m_iPassTick;
	int m_iWheelSize;
	int m_iIndex;
	int m_iMaxCheckTimer;// 측정 최대 시간
	std::vector<std::list<TWHEEL*>> m_vecTimerWheel;

	int m_iUpdateTick;

	void Update();

	LARGE_INTEGER freq;

public:
	std::list<TWHEEL*> m_listRemainTimeEvent;			// 더 놀아야할 시간이 있는 리스트
	std::list<TWHEEL*> m_listCompleteTimeEvent;			// 처리가 완료된 리스트
	void UpdateTick();

	int GetTick() { return m_iTick; }
	int GetWheelSize() { return m_iWheelSize; }
	int GetRoundTick() { return m_iTick * m_iWheelSize; }
	int GetNowIndex() { return m_iIndex; }

	void SetMaxCheckTimer(int MaxTime) { m_iMaxCheckTimer = MaxTime; }
	int GetMaxCheckTimer() { return m_iMaxCheckTimer; }

	void Push(TWHEEL* data);
	void RePush(TWHEEL* data);
	bool Pop(TWHEEL* data);
	void Replace(TWHEEL* data);

	CTimerWheel operator++();
};

