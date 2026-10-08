#pragma once
#include "CTimerWheel.h"

class CTimerWheelManager
{
public:
	CTimerWheelManager();
	virtual ~CTimerWheelManager();

	void Update();

public:
	void Push(TWHEEL* data);
	void RePush(TWHEEL* data);
	void Pop(TWHEEL* data);
	bool Replace(TWHEEL* data, int subTime, bool bAdd = true);
	bool ChangeTime(TWHEEL* data, int time);

	void AddTimerWheel(int tick, int wheelsize);
	virtual void PostProcessing();			// 타이머 다돈거 후처리

	int GetCompleteDataCount() const { return m_vecCompleteData.size(); }
    const std::vector<TWHEEL*>& GetCompleteData() const { return m_vecCompleteData; }
    void ClearCompleteData() { m_vecCompleteData.clear(); }
private:
	std::vector<TWHEEL*> m_vecCompleteData;
	double m_dRemainTime;

	std::vector< CTimerWheel*> m_vecTimerWheel;
	int	m_iTimerWheelCount;

	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_endTime;
	LARGE_INTEGER m_frequency;
};

