#pragma once
#include "CTimerWheel.h"

class CTimerWheelManager
{
public:
	CTimerWheelManager();
	~CTimerWheelManager();

	void Update();

public:
	void Push(TWHEEL* data);
	void RePush(TWHEEL* data);
	void Pop(TWHEEL* data);
	bool Replace(TWHEEL* data, int subTime, bool bAdd = true);
	bool ChangeTime(TWHEEL* data, int time);

	void AddTimerWheel(int tick, int wheelsize);
	virtual void PostProcessing();			// 타이머 다돈거 후처리

	int GetCompleteDataCount() { return m_vecCompleteData.size(); }
	std::vector<TWHEEL*> m_vecCompleteData;
private:
	unsigned int m_iLastUpdateTime;
	int m_iUpdateTick = 1;
	double m_dRemainTime;

	std::vector< CTimerWheel*> m_vecTimerWheel;
	int	m_vecTimerWheelSize;

	LARGE_INTEGER m_LStartTime;
	LARGE_INTEGER m_LEndTime;
	LARGE_INTEGER m_Lfreq;
};

