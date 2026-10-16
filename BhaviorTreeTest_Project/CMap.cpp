#include "CMap.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <vector>
#include <algorithm>
#include <climits>

#include "CDoor.h"

//#define DEBUG_DUMMY_DATA

HPEN g_BlackPen;

CMap::CMap()
{
	m_astar.SetMapInit(MAP_SIZE_WH, MAP_SIZE_WH);
	g_BlackPen = (HPEN)CreatePen(BS_SOLID, 1, RGB(0, 0, 0));

	std::ifstream fp("SimulatorMap.txt");
	if (!fp.is_open())
		printf("Failed OPEN MAP\n");

	std::string line;

	for (int y = 0; y < MAP_SIZE_WH; y++)
	{
		std::getline(fp, line);

		std::stringstream ss(line);
		std::string token;
		for (int x = 0; x < MAP_SIZE_WH; x++)
		{
			std::getline(ss, token, ',');
			m_vecMapStat[y][x].iType = atoi(token.c_str());
			int iType = m_vecMapStat[y][x].iType;

			if (iType == eSTAT_TYPE::Wall)
			{
				m_vecMapStat[y][x].bIsMove = false;
				m_astar.SetWall(x, y);
			}
			else
				m_vecMapStat[y][x].bIsMove = true;

			if (iType == eSTAT_TYPE::Wall || iType == eSTAT_TYPE::Door)
				m_vecMapStat[y][x].bIsBuildWall = true;
			else
				m_vecMapStat[y][x].bIsBuildWall = false;

			m_vecMapStat[y][x].iRegionIndex = -1; // 초기값
		}
	}

	fp.close();

	SearchRegion();

	m_stSpawnPoint[0] = st_Pos{ 0, 0 };
	m_stSpawnPoint[1] = st_Pos{ 17, 21 };
	m_stSpawnPoint[2] = st_Pos{ 110, 87 };
}

CMap::~CMap()
{
	int nLoop = m_vecRegion.size();
	for (int i = 0; i < nLoop; i++)
	{
		delete m_vecRegion[i];
	}
}

void CMap::Update()
{
}

void CMap::Render(HDC hdc)
{
	RenderGrid(hdc);

	int nLoop = m_vecRegion.size();
	for (int i = 0; i < nLoop; i++)
		m_vecRegion[i]->Render(hdc);
}

void CMap::RenderGrid(HDC hdc)
{
	SelectObject(hdc, g_BlackPen);
	int x = 0, y = 0;
	for (int i = 0; i < MAP_SIZE_WH; i ++)
	{
		MoveToEx(hdc, x, 0, NULL);
		LineTo(hdc, x, MAP_SIZE_WH * m_iGridRenderSize);
		x += m_iGridRenderSize;

		MoveToEx(hdc, 0, y, NULL);
		LineTo(hdc, MAP_SIZE_WH * m_iGridRenderSize, y);
		y += m_iGridRenderSize;
	}
}

CRegion* CMap::GetRegion(int index)
{
	if (index < 0 || index >= static_cast<int>(m_vecRegion.size()))
		return nullptr;

	return m_vecRegion[index];
}

st_Pos CMap::GetSpawnPos(int index)
{
	if (index < 0 || 2 < index)
		return m_stSpawnPoint[0];

	return m_stSpawnPoint[index];
}

int CMap::GetNearRegionIndex(st_Pos pos)
{
    int bestIndex = -1;
    long long bestDistance = LLONG_MAX;
    for (CRegion* pRegion : m_vecRegion)
    {
        const st_Pos center = pRegion->GetPos();
        const long long dx = static_cast<long long>(pos.X) - center.X;
        const long long dy = static_cast<long long>(pos.Y) - center.Y;
        const long long distance = dx * dx + dy * dy;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = pRegion->GetRegionIndex();
        }
    }
    return bestIndex;
}

int CMap::GetNearRegion(const std::vector<int>& vec, st_Pos pos)
{
	int min = 99999;
	int ret = -1;
	for (int i = 0; i < vec.size(); i++)
	{
		int index = vec[i];
		int length = pos.LengthLength(m_vecRegion[index]->GetPos());
		if (min > length)
		{
			min = length;
			ret = index;
		}
	}

	return ret;
}

CPartsBase* CMap::GetPart(st_Pos pos)
{
	__int64 key = ((__int64)pos.Y << 32 | pos.X);

    const auto it = m_mapParts.find(key);
    return it != m_mapParts.end() ? it->second : nullptr;
}

int CMap::GetRegionOppositeCount(int regionindex)
{
	CRegion* pRegion = GetRegion(regionindex);
	if (pRegion == nullptr)
		return 0;

	return pRegion->GetOppositeCount();
}

bool CMap::FindShape(int sX, int sY, st_Rect& region, std::vector<st_Part>& vecParts)
{
	bool bSearch = false;
	int iX = sX;
	for (; iX < MAP_SIZE_WH; iX++)
	{
		if (m_vecMapStat[sY][iX].iType == eSTAT_TYPE::Door)
		{
			st_Part part;
			CreateDoor(part, eDirection::North, st_Pos{ iX, sY });
			vecParts.push_back(part);
		}
		//┐  찾기
		if (m_vecMapStat[sY][iX].iType == eSTAT_TYPE::None &&
			m_vecMapStat[sY][iX - 1].bIsBuildWall == true)
		{
			break;
		}
	}

	iX--;
	if (
		m_vecMapStat[sY][iX].bIsBuildWall == true &&
		m_vecMapStat[sY][iX - 1].bIsBuildWall == true &&
		m_vecMapStat[sY + 1][iX].bIsBuildWall == true)
	{
		bSearch = true;
	}

	if (!bSearch)
		return false;

	bSearch = false;
	int iY = sY;
	for (; iY < MAP_SIZE_WH; iY++)
	{
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::Door)
		{
			st_Part part;
			CreateDoor(part, eDirection::East, st_Pos{ iX, iY });
			vecParts.push_back(part);
		}
		// ┘ 찾기
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::None &&
			m_vecMapStat[iY - 1][iX].bIsBuildWall == true)
		{
			break;
		}
	}

	iY--;
	if (
		m_vecMapStat[iY][iX].bIsBuildWall == true &&
		m_vecMapStat[iY][iX - 1].bIsBuildWall == true &&
		m_vecMapStat[iY - 1][iX].bIsBuildWall == true)
	{
		bSearch = true;
	}
	region.Right = iX;
	region.Bottom = iY;
	if (!bSearch)
		return false;

	bSearch = false;

	for (; iX >= 0; iX--)
	{
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::Door)
		{
			st_Part part;
			CreateDoor(part, eDirection::South, st_Pos{iX, iY });
			vecParts.push_back(part);
		}
		// └ 찾기
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::None &&
			m_vecMapStat[iY][iX + 1].bIsBuildWall == true)
		{
			break;
		}
	}

	iX++;
	if (
		m_vecMapStat[iY][iX].bIsBuildWall == true &&
		m_vecMapStat[iY][iX + 1].bIsBuildWall == true &&
		m_vecMapStat[iY - 1][iX].bIsBuildWall == true)
	{
		bSearch = true;
	}

	if (!bSearch)
		return false;

	for (; iY >= 0; iY--)
	{
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::Door)
		{
			st_Part part;
			CreateDoor(part, eDirection::West, st_Pos{ iX, iY });
			vecParts.push_back(part);
		}
		// └ 찾기
		if (m_vecMapStat[iY][iX].iType == eSTAT_TYPE::None)
		{
			break;
		}
	}

	return true;
}

bool CMap::SearchRegion()
{
	int index = 0;
	// Corner detection reads the next row, so the last row cannot be a candidate.
	for (int iY = 1; iY < MAP_SIZE_WH - 1; iY++)
	{
		for (int iX = 0; iX < MAP_SIZE_WH-1; iX++)
		{
			//┌  찾기
			if (m_vecMapStat[iY][iX].bIsMove == false &&
				m_vecMapStat[iY][iX].bIsBuildWall == true &&
				m_vecMapStat[iY][iX + 1].bIsBuildWall == true &&
				m_vecMapStat[iY + 1][iX].bIsBuildWall == true)
			{
				std::vector<st_Part> vecParts;
				st_Rect region;
				region.Left = iX;
				region.Top = iY;
				bool ret = FindShape(iX, iY, region, vecParts);
				if (ret)
				{
					st_Rect _rect(region.Left, region.Top, region.Right, region.Bottom);
					CRegion* pRegion = new CRegion(_rect, m_iGridRenderSize, index);
					m_vecRegion.push_back(pRegion);
					AddPart(vecParts, index);

					for (int y = _rect.Top; y <= _rect.Bottom; y++)
					{
						for (int x = _rect.Left; x <= _rect.Right; x++)
						{
							m_vecMapStat[y][x].iRegionIndex = index;
						}
					}

					++index;
					printf("Find Region Area Size %d * %d = %d\n", _rect.Width, _rect.Height, _rect.Width * _rect.Height);
				}
			}
		}
	}

#ifdef DEBUG_DUMMY_DATA
	m_vecRegion[0]->DebugFunc_Test(eTeam::Red, eTeam::None,0,0,0,0);	//Red 점령
	m_vecRegion[1]->DebugFunc_Test(eTeam::Blue, eTeam::None,0,0,0,0);	//Blue 점령

	m_vecRegion[2]->DebugFunc_Test(eTeam::None, eTeam::Red, 2, 1, 0, 0);	// 비 점령 구역 레드 차지중
	m_vecRegion[3]->DebugFunc_Test(eTeam::None, eTeam::Blue, 2, 1, 0, 0);	// 비 점령 구역 블루 차지중

	m_vecRegion[4]->DebugFunc_Test(eTeam::Red, eTeam::Blue, 3, 2, 0, 0); // Red 점령 Blue 뺐는중
	m_vecRegion[5]->DebugFunc_Test(eTeam::Blue, eTeam::Red, 2, 3, 0, 0); // Blue 점령 Red 뺐는중 Blue 인원 더 많음
#endif // DEBUG_DUMMY_DATA

	return true;
}

void CMap::AddPart(std::vector<st_Part>& vec, int regionindex)
{
	int nLoop = vec.size();
	for (int i = 0; i < nLoop; i++)
	{
		int type = vec[i].type;
		CPartsBase* pPart = nullptr;
		switch (type)
		{
		case eSTAT_TYPE::Door:
		{
			pPart = new CDoor();
			pPart->SetPos(vec[i].pos);

			__int64 key = ((__int64)vec[i].pos.Y << 32 | vec[i].pos.X);
			m_mapParts.insert(std::make_pair(key, pPart));

			m_vecRegion[regionindex]->AddParts(pPart);
		}
		break;
		default:
			break;
		}

	}
}



bool CMap::AllRegionOccupy()
{
	for (CRegion* pRegion : m_vecRegion)
	{
		if (pRegion->GetOccupyTeam() == eTeam::None)
		{
			return false;
		}
	}
	return true;
}

void CMap::CreateDoor(st_Part& out, eDirection dir, st_Pos pos)
{
	out.type = eSTAT_TYPE::Door;
	out.Dir = dir;
	out.pos = pos;
}

bool CMap::BresenhamLine(int sX, int sY, int eX, int eY)
{
	int CX = sX, CY = sY;

	int Dx = abs(eX - sX); // x 변화량
	int Dy = abs(eY - sY); // y 변화량

	int Mx = eX > sX ? 1 : -1;  // x 이동량
	int My = eY > sY ? 1 : -1;  // y 이동량

	int BresenhamValue = Dx - Dy;

	// Bresenham
	while (1)
	{
		if (CX == eX && CY == eY) break;

		int BresenhamValue2 = 2 * BresenhamValue;
		if (BresenhamValue2 > -Dy) // x변화량이 크다는것을 의미
		{
			BresenhamValue -= Dy;
			CX += Mx;
		}
		if (BresenhamValue2 < Dx) // y변화량이 크다는것을 의미
		{
			BresenhamValue += Dx;
			CY += My;
		}
		if (m_vecMapStat[CY][CX].bIsMove == false)
			return false;
	}
	return true;
}

void CMap::BresenhamLine(int sX, int sY, int eX, int eY, std::queue<st_PosInfo>& out)
{
	int CX = sX, CY = sY;

	int Dx = abs(eX - sX); // x 변화량
	int Dy = abs(eY - sY); // y 변화량

	int Mx = eX > sX ? 1 : -1;  // x 이동량
	int My = eY > sY ? 1 : -1;  // y 이동량

	int BresenhamValue = Dx - Dy;

	while(!out.empty())
	{
		out.pop();
	}

	// Bresenham
	while (1)
	{
		if (CX == eX && CY == eY) break;

		int BresenhamValue2 = 2 * BresenhamValue;
		if (BresenhamValue2 > -Dy) // x변화량이 크다는것을 의미
		{
			BresenhamValue -= Dy;
			CX += Mx;
		}
		if (BresenhamValue2 < Dx) // y변화량이 크다는것을 의미
		{
			BresenhamValue += Dx;
			CY += My;
		}
		if (m_vecMapStat[CY][CX].bIsMove == false)
			return ;

		__int64 checkkey = ((__int64)CY << 32 | CX);
		int type = 0;

        const auto it = m_mapParts.find(checkkey);
        if (it != m_mapParts.end())
        {
            CPartsBase* pPart = it->second;
			type = pPart->GetType();
		}

		out.push(st_PosInfo{type, st_Pos(CX, CY)});
	}
	return ;
}

bool CMap::OnCollision(CEntity* pOwner, st_Pos pos, int Damage)
{
	if (m_vecMapStat[pos.Y][pos.X].iType == eSTAT_TYPE::Wall)
		return true;
	return false;
}
