#pragma once
#include "CPartsBase.h"

class CDoor : public CPartsBase
{
public:
	CDoor();
private:
	bool m_bOpen;

	HBRUSH m_hOpenBrush;
	HBRUSH m_hCloseBrush;

public:

	// CPartsBase을(를) 통해 상속됨
	void Update() override;
	void Render(HDC hdc) override;
	bool Use() override;
	bool Destory() override;

	bool GetOpen() const { return m_bOpen; }
};
