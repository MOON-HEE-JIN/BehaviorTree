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

	LARGE_INTEGER m_frequency;

	friend class CTimerWheelManager;
	std::list<TWHEEL*> m_listRemainTimeEvent;			// 더 놀아야할 시간이 있는 리스트
	std::list<TWHEEL*> m_listCompleteTimeEvent;			// 처리가 완료된 리스트
public:
	void UpdateTick();

	int GetTick() const { return m_iTick; }
	int GetWheelSize() const { return m_iWheelSize; }
	int GetRoundTick() const { return m_iTick * m_iWheelSize; }
	int GetNowIndex() const { return m_iIndex; }

	void SetMaxCheckTimer(int MaxTime) { m_iMaxCheckTimer = MaxTime; }
	int GetMaxCheckTimer() const { return m_iMaxCheckTimer; }

	void Push(TWHEEL* data);
	void RePush(TWHEEL* data);
	bool Pop(TWHEEL* data);
	void Replace(TWHEEL* data);

	CTimerWheel& operator++();
};

