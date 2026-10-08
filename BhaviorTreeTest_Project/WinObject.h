#pragma once

#include <Windows.h>
#include "Defines.h"

class WinObject
{
private:
	int m_iUpdateTime = 1000;
	int m_iRenderSize = 8;
	int m_iRenderHalfSize = 4;

	int m_iRenderPrivateSize = 0;
	int m_iRenderPrivateHalfSize = 0;

	bool m_bEnable = true;
protected:
	st_Pos m_stPos;
	st_Pos m_stRenderPos;

	HBRUSH m_hRenderBrush = nullptr;
public:
	virtual ~WinObject() = default;
	virtual void Update() = 0;
	virtual void CheckCoolTime(int type) {};
	virtual void CheckDuringTime(int type) {};
	virtual void Render(HDC hdc) = 0;
	virtual void Render_Layer_01(HDC hdc) {}; // Render 보다 먼저 그려짐

public:
	void SetUpdateTime(int time) { m_iUpdateTime = time; }
	void SetRenderPrivateSize(int size) { m_iRenderPrivateSize = size; m_iRenderPrivateHalfSize = static_cast<int>(m_iRenderPrivateSize * 0.5f); }
	void SetEnable(bool enable) { m_bEnable = enable; }
	void SetPos(const st_Pos& pos) {
		m_stPos = pos;
		m_stRenderPos.X = m_stPos.X * m_iRenderSize;
		m_stRenderPos.Y = m_stPos.Y * m_iRenderSize;
	}

	int GetUpdateTime() const { return m_iUpdateTime; }

	int GetRenderSize() const { return m_iRenderSize; }
	int GetRenderHalfSize() const { return m_iRenderHalfSize; }

	int GetRenderPrivateSize() const { return m_iRenderPrivateSize; }
	int GetRenderPrivateHalfSize() const { return m_iRenderPrivateHalfSize; }

	bool GetEnable() const { return m_bEnable; }
	st_Pos GetPos() const { return m_stPos; }

};
