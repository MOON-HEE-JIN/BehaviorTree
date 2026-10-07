#include "CDoor.h"


CDoor::CDoor()
{
    this->type = eSTAT_TYPE::DOOR;
    m_bOpen = false;

    m_hOpenBrush = CreateSolidBrush(RGB(37, 37, 38));
    m_hCloseBrush = CreateSolidBrush(RGB(0, 255, 0));
}

void CDoor::Update()
{}

void CDoor::Render(HDC hdc)
{
    m_bOpen ? SelectObject(hdc, m_hOpenBrush) : SelectObject(hdc, m_hCloseBrush);
    Ellipse(hdc
        , m_stRenderPos.X - GetRenderHalfSize()
        , m_stRenderPos.Y - GetRenderHalfSize()
        , m_stRenderPos.X + GetRenderHalfSize()
        , m_stRenderPos.Y + GetRenderHalfSize());
}

bool CDoor::Use()
{
    m_bOpen = !m_bOpen;
    m_bMove = m_bOpen;

    return true;
}

bool CDoor::Destory()
{
    return true;
}
