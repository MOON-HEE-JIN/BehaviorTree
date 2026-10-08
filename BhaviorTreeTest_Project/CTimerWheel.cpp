#include "CTimerWheel.h"
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <process.h>

CTimerWheel::CTimerWheel(int tick, int wheelsize)
{
	m_iTick = tick;
	m_iPassTick = 500;
	m_iWheelSize = wheelsize;
	m_iIndex = 0;
	m_iUpdateTick = 0;

	m_vecTimerWheel.resize(wheelsize);
	QueryPerformanceFrequency(&m_frequency);
}

CTimerWheel::~CTimerWheel()
{
}

void CTimerWheel::Update()
{
	if (++m_iIndex >= m_iWheelSize)
		m_iIndex = 0;

	std::list<TWHEEL*>::iterator biter = m_vecTimerWheel[m_iIndex].begin();
	std::list<TWHEEL*>::iterator eiter = m_vecTimerWheel[m_iIndex].end();
	for (biter; biter != eiter; )
	{
		bool ret = (*biter)->TimeCheck();

		if (ret) // 실행됨
		{
			if ((*biter)->RemaindTime > m_iPassTick)	// m_iPassTick 보다 크면 시간을 다시 잰다
			{
				(*biter)->TotalTimer = (*biter)->RemaindTime;
				m_listRemainTimeEvent.push_back((*biter));
			}
			else
			{
				(*biter)->Update(m_frequency);
				m_listCompleteTimeEvent.push_back(*biter);
			}
			biter = m_vecTimerWheel[m_iIndex].erase(biter);
		}
		else // 실행 안됨
		{
			++biter;
		}
	}
}

void CTimerWheel::UpdateTick()
{
	m_iUpdateTick++;
	if (m_iUpdateTick >= m_iTick)
	{
		m_iUpdateTick = 0;
		Update();
	}
}

void CTimerWheel::Push(TWHEEL* data)
{
	if (data->TotalTimer < 0)
		return;

	int checkTick = data->TotalTimer / m_iTick;
	data->RemaindTime = m_iUpdateTick;// 1ms 기준으로 쌓이는 시간(타이머가 이미체크한 시간)

	int offset = checkTick % m_iWheelSize;
	int inputIndex = (m_iIndex + offset) % m_iWheelSize;

	data->Round = checkTick / m_iWheelSize;

	if (offset != 0)
		data->Round++;

	m_vecTimerWheel[inputIndex].push_back(data);
	data->Index = inputIndex;
	data->PushTime = GetTickCount();
#ifdef __DEBUG__
	QueryPerformanceCounter(&data->DebugTime);
#endif // __DEBUG__
}

void CTimerWheel::RePush(TWHEEL* data)
{
	int checkTick = data->TotalTimer / m_iTick;
	data->RemaindTime = m_iUpdateTick;// 1ms 기준으로 쌓이는 시간(타이머가 이미체크한 시간)

	int offset = checkTick % m_iWheelSize;
	int inputIndex = (m_iIndex + offset) % m_iWheelSize;

	data->Round = checkTick / m_iWheelSize;

	if (offset != 0)
		data->Round++;

	m_vecTimerWheel[inputIndex].push_back(data);
	data->Index = inputIndex;
	data->PushTime = GetTickCount();
}

bool CTimerWheel::Pop(TWHEEL* data)
{
	std::list<TWHEEL*>::iterator biter = m_vecTimerWheel[data->Index].begin();
	std::list<TWHEEL*>::iterator eiter = m_vecTimerWheel[data->Index].end();
	for (biter; biter != eiter; ++biter)
	{
		if ((*biter) == data)
		{
			m_vecTimerWheel[data->Index].erase(biter);
			return true;
		}
	}
	return false;
}

void CTimerWheel::Replace(TWHEEL* data)
{
	int checkTick = data->TotalTimer / m_iTick;
	data->RemaindTime = m_iUpdateTick;// 1ms 기준으로 쌓이는 시간(타이머가 이미체크한 시간)

	int offset = checkTick % m_iWheelSize;
	int inputIndex = (m_iIndex + offset) % m_iWheelSize;

	data->Round = checkTick / m_iWheelSize;

	if (offset != 0)
		data->Round++;

	m_vecTimerWheel[inputIndex].push_back(data);
	data->Index = inputIndex;
}

CTimerWheel& CTimerWheel::operator++()
{
	m_iUpdateTick++;
	if (m_iUpdateTick >= m_iTick)
	{
		m_iUpdateTick = 0;
		Update();
	}
	return *this;
}

bool st_TimerWheel::TimeCheck()
{
	Round--;
	if (Round <= 0)
	{
		return true;
	}
	else
		return false;
}

bool st_TimerWheel::Update(LARGE_INTEGER freq)
{
#ifdef __DEBUG__
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	double ms = (double)(end.QuadPart - DebugTime.QuadPart) * 1000 / (double)freq.QuadPart;
	//printf("UPDATE %d  DebugTime %.2f[S] \n", TotalTimer, ms/1000.f);
#endif // __DEBUG__
	return true;
}
