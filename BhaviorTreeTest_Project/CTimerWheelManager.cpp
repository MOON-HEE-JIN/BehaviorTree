#include "CTimerWheelManager.h"
#include <algorithm>

CTimerWheelManager::CTimerWheelManager()
{
	QueryPerformanceFrequency(&m_Lfreq);
	m_LStartTime = { 0 };
	m_dRemainTime = 0;

	m_vecTimerWheelSize = 0;

	AddTimerWheel(1, 1000);
	AddTimerWheel(1000, 60);
	AddTimerWheel(60000, 60);
}

CTimerWheelManager::~CTimerWheelManager()
{
}

void CTimerWheelManager::Update()
{
	double ms = 1;
	if (m_LStartTime.QuadPart != 0)
	{
		LARGE_INTEGER temp = m_LStartTime;
		QueryPerformanceCounter(&m_LEndTime);
		m_LStartTime = m_LEndTime;
		ms = (double)(m_LEndTime.QuadPart - temp.QuadPart) * 1000 / (double)m_Lfreq.QuadPart;
	}
	else
		QueryPerformanceCounter(&m_LStartTime);

	ms += m_dRemainTime;
	int nLoop = static_cast<int>(ms);
	m_dRemainTime = ms - nLoop;
	std::list<TWHEEL*>::iterator biter;
	std::list<TWHEEL*>::iterator eiter;
#ifdef _DEBUG
	if (nLoop > 10)
		nLoop = 10;
#endif // DEBUG

	for (int i = 0; i < nLoop; i++)
	{
		++(*m_vecTimerWheel[0]);
		for (int j = 1; j < m_vecTimerWheelSize; j++)
		{
			++(*m_vecTimerWheel[j]);
			biter = m_vecTimerWheel[j]->m_listRemainTimeEvent.begin();
			eiter = m_vecTimerWheel[j]->m_listRemainTimeEvent.end();
			for (biter; biter != eiter; biter++)
				m_vecTimerWheel[j-1]->Replace(*biter);
			m_vecTimerWheel[j]->m_listRemainTimeEvent.clear();
		}
		
	}

	PostProcessing();
}

void CTimerWheelManager::Push(TWHEEL* data)
{
	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		if (data->TotalTimer > m_vecTimerWheel[i]->GetMaxCheckTimer())
			continue;

		m_vecTimerWheel[i]->Push(data);
		break;
	}
}

void CTimerWheelManager::RePush(TWHEEL* data)
{
	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		if (data->TotalTimer > m_vecTimerWheel[i]->GetMaxCheckTimer())
			continue;

		m_vecTimerWheel[i]->RePush(data);
		break;
	}
}

void CTimerWheelManager::Pop(TWHEEL* data)
{
	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		if (data->TotalTimer > m_vecTimerWheel[i]->GetMaxCheckTimer())
			continue;

		m_vecTimerWheel[i]->Pop(data);
	}
}

bool CTimerWheelManager::Replace(TWHEEL* data, int subTime, bool bAdd)
{
	CTimerWheel* pTimerWheel = nullptr;
	
	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		if (data->TotalTimer > m_vecTimerWheel[i]->GetMaxCheckTimer())
			continue;

		pTimerWheel = m_vecTimerWheel[i];
		break;
	}
	if (pTimerWheel == nullptr)
		return false;

	bool ret = pTimerWheel->Pop(data);
	if (!ret)
		return false;

	unsigned int RemaindTime = 0;
	int curTime = GetTickCount();
	if(bAdd)
		RemaindTime = data->TotalTimer - (GetTickCount() - data->PushTime) + subTime;
	else
		RemaindTime = data->TotalTimer - (GetTickCount() - data->PushTime) - subTime;

	if (RemaindTime <= 0)
	{
		pTimerWheel->m_listCompleteTimeEvent.push_back(data);
		return false;
	}
	
	data->TotalTimer = RemaindTime;
	RePush(data);
	return true;
}

bool CTimerWheelManager::ChangeTime(TWHEEL* data, int time)
{
	CTimerWheel* pTimerWheel = nullptr;

	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		if (data->TotalTimer > m_vecTimerWheel[i]->GetMaxCheckTimer())
			continue;

		pTimerWheel = m_vecTimerWheel[i];
		break;
	}
	if (pTimerWheel == nullptr)
		return false;

	bool ret = pTimerWheel->Pop(data);
	if (!ret)
		return false;

	unsigned int RemaindTime = time;
	
	if (RemaindTime <= 0)
	{
		pTimerWheel->m_listCompleteTimeEvent.push_back(data);
		return false;
	}

	data->TotalTimer = RemaindTime;
	RePush(data);
	return true;
}

void CTimerWheelManager::AddTimerWheel(int tick, int wheelsize)
{
	CTimerWheel* pTimerWheel = new CTimerWheel(tick, wheelsize);

	m_vecTimerWheel.push_back(pTimerWheel);
	std::sort(m_vecTimerWheel.begin(), m_vecTimerWheel.end(),
		[](CTimerWheel* p1, CTimerWheel* p2) 
		{
			return p1->GetRoundTick() < p2->GetRoundTick();
		} 
	);
	m_vecTimerWheelSize++;

	m_vecTimerWheel[m_vecTimerWheelSize - 1]->SetMaxCheckTimer(m_vecTimerWheel[m_vecTimerWheelSize - 1]->GetRoundTick() * 60);
	for (int i =0; i < m_vecTimerWheelSize-1; i++)
	{
		m_vecTimerWheel[i]->SetMaxCheckTimer(m_vecTimerWheel[i + 1]->GetRoundTick());	// 최대 측정 값 지정
	}

}

void CTimerWheelManager::PostProcessing()
{
	std::list<TWHEEL*>::iterator biter;
	std::list<TWHEEL*>::iterator eiter;
	for (int i = 0; i < m_vecTimerWheelSize; i++)
	{
		biter = m_vecTimerWheel[i]->m_listCompleteTimeEvent.begin();
		eiter = m_vecTimerWheel[i]->m_listCompleteTimeEvent.end();
		for (biter; biter != eiter; ++biter)
			m_vecCompleteData.push_back(*biter);
		m_vecTimerWheel[i]->m_listCompleteTimeEvent.clear();
	}
}
