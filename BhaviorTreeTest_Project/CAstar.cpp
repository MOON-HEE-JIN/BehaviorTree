#include "CAstar.h"
#include <algorithm>

#define ASTAR_GRID_SIZE 1
using namespace GridColor;

bool desc(MAP_STAT_NODE* a, MAP_STAT_NODE* b)
{
	return a->node->F < b->node->F;
}

CAstarGrid::CAstarGrid()
{
}

CAstarGrid::~CAstarGrid()
{
	for (int Y = 0; Y < m_iMapHeight; Y++)
	{
		for (int X = 0; X < m_iMapWidth; X++)
		{
			if (m_vecMapStat[Y][X].wall || m_vecMapStat[Y][X].node == nullptr)
				continue;

			delete m_vecMapStat[Y][X].node;
		}
	}
}

bool CAstarGrid::GetWall(int X, int Y)
{
	if (X >= m_iMapWidth || Y >= m_iMapHeight ||
		X < 0 || Y < 0)
		return true;

	return m_vecMapStat[Y][X].wall;
}

MAP_STAT_NODE* CAstar::CreateGridNode(GridColor::Dir _dir, int _X, int _Y, GridNode* pParent, float _G)
{
	float H = static_cast<float>(GetH(_X, _Y));
	MAP_STAT_NODE* pNode = &m_vecMapStat[_Y][_X];
	if (pNode->node == nullptr)
	{
		pNode->node = new GridNode(_dir, _X, _Y, _G, H, _G + H, pParent);
	}
	else if (pNode->node->G == 0)
	{
		pNode->node->dir = _dir;
		pNode->node->G = _G;
		pNode->node->H = H;
		pNode->node->F = _G + H;
		pNode->node->parent = pParent;
	}
	else
	{
		if (pNode->node->G <= _G)
			return nullptr;

		// pNode 가 OpenList 에 있을 경우 가중치 교체 만 해주고 nullptr 반환 (이미 OpenList에 존재)
		pNode->node->G = _G;
		pNode->node->H = H;
		pNode->node->F = _G + pNode->node->H;
		pNode->node->parent = pParent;
		return nullptr;
	}
	return pNode;
}
void CAstar::InsertOpenList(MAP_STAT_NODE* pNode)
{
	if (pNode == nullptr)
		return;

	OpenList.push_back(pNode);
}
float CAstarGrid::GetH(int X, int Y)
{
	return static_cast<float>(abs(m_iEndX - X) + abs(m_iEndY - Y));
}


void CAstarGrid::ClearAstarNode()
{
	for (int Y = 0; Y < m_iMapHeight; Y++)
	{
		for (int X = 0; X < m_iMapWidth; X++)
		{
			if (m_vecMapStat[Y][X].node == nullptr || m_vecMapStat[Y][X].wall)
				continue;
			m_vecMapStat[Y][X].node->Clear();
		}
	}
}

void CAstarGrid::SetMapInit(int world_width, int world_height)
{
	//Grid 가로, 세로 길이
	m_iMapWidth = world_width / ASTAR_GRID_SIZE;
	m_iMapHeight = world_height / ASTAR_GRID_SIZE;

	SetMapVectorInit();
}

void CAstarGrid::SetWall(int x, int y)
{
	m_vecMapStat[y][x].wall = true;
}

void CAstarGrid::SetWall(int x, int y, bool bWall)
{
	m_vecMapStat[y][x].wall = bWall;
}

void CAstarGrid::SetMapVectorInit()
{
	m_vecMapStat.resize(m_iMapHeight);
	for (int i = 0; i < m_iMapHeight; i++)
		m_vecMapStat[i].resize(m_iMapWidth);
}

CAstar::CAstar()
{
}

CAstar::CAstar(int _width, int _height, int _grid)
{
	SetMapInit(_width, _height);
}

CAstar::~CAstar()
{

}

void CAstar::listSort()
{
	OpenList.sort(desc);
}

bool CAstar::FindPath(int s_X, int s_Y, int e_X, int e_Y, std::vector<st_Pos>& vec)
{
	if (s_X == e_X && s_Y == e_Y)
		return false;

	m_iEndX = e_X; m_iEndY = e_Y;
	m_iStartX = s_X; m_iStartY = s_Y;

	float H = GetH(m_iStartX, m_iStartY);

	//PathFinder Start Node
	GridNode* pNode = m_vecMapStat[m_iStartY][m_iStartX].node;

	ListClear();

	if (pNode == nullptr)
	{
		m_vecMapStat[m_iStartY][m_iStartX].node = new GridNode(Dir::Left, m_iStartX, m_iStartY, 0, H, H, nullptr);
	}
	else
	{
		pNode->Clear();
		pNode->dir = Dir::Left;
		pNode->G = 0;
		pNode->H = H;
		pNode->F = H;
		pNode->parent = nullptr;
	}

	// PathFinder 시작 노드 기준으로 8 방향 탐색 하여 OpenList 에 추가
	JT_CreateStartNode(&m_vecMapStat[m_iStartY][m_iStartX]);

	bool ret = false;
	while (1)
	{
		MAP_STAT_NODE* pNode = (*OpenList.begin());
		OpenList.pop_front();
		CloseList.push_back(pNode);

		if (pNode == nullptr)
		{
			ret = false;
			break;
		}

		if (pNode->node->X == m_iEndX && pNode->node->Y == m_iEndY)
		{
			ResultPath(vec);
			ret = true;
			break;
		}

		JT_SerchCreateNode(pNode);

		if (OpenList.empty())
		{
			ret = false;
			break;
		}
		listSort();
	}
	ClearAstarNode();
	return ret;
}

void CAstar::ResultPath(std::vector<st_Pos>& vec)
{
	vec.clear();
	vec.push_back(st_Pos(m_iEndX, m_iEndY));

	GridNode* pNode = m_vecMapStat[m_iEndY][m_iEndX].node;

	while (pNode->parent != nullptr)
	{
		pNode = pNode->parent;

		vec.push_back(st_Pos(pNode->X, pNode->Y));
	}

	vec.pop_back(); // 시작점 제거
}

bool CAstar::isStraight(int x1, int y1, int x2, int y2)
{
	int CX = x1, CY = y1;

	int Dx = abs(x2 - x1); // x 변화량
	int Dy = abs(y2 - y1); // y 변화량

	int Mx = x2 > x1 ? 1 : -1;  // x 이동량
	int My = y2 > y1 ? 1 : -1;  // y 이동량

	int BresenhamValue = Dx - Dy;

	// Bresenham drawing
	while (1)
	{
		if (CX == x2 && CY == y2) return true;

		if (GetWall(CX, CY)) return false;

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
	}
}

void CAstar::JT_CreateAroundNode(MAP_STAT_NODE* pNode)
{
	int X = pNode->node->X;
	int Y = pNode->node->Y;
	if (!GetWall(X, Y + 1))
		InsertOpenList(CreateGridNode(Dir::Down, X, Y + 1, pNode->node, HV));

	if (!GetWall(X, Y - 1))
		InsertOpenList(CreateGridNode(Dir::Up, X, Y - 1, pNode->node, HV));

	if (!GetWall(X + 1, Y + 1))
		InsertOpenList(CreateGridNode(Dir::RightDown, X + 1, Y + 1, pNode->node, D));

	if (!GetWall(X + 1, Y - 1))
		InsertOpenList(CreateGridNode(Dir::RightUp, X + 1, Y - 1, pNode->node, D));

	if (!GetWall(X + 1, Y))
		InsertOpenList(CreateGridNode(Dir::Right, X + 1, Y, pNode->node, HV));

	if (!GetWall(X - 1, Y + 1))
		InsertOpenList(CreateGridNode(Dir::LeftDown, X - 1, Y + 1, pNode->node, D));

	if (!GetWall(X - 1, Y - 1))
		InsertOpenList(CreateGridNode(Dir::LeftUp, X - 1, Y - 1, pNode->node, D));

	if (!GetWall(X - 1, Y))
		InsertOpenList(CreateGridNode(Dir::Left, X - 1, Y, pNode->node, HV));

}

void CAstar::JT_SerchCreateNode(MAP_STAT_NODE* pNode)
{
	int X = pNode->node->X;
	int Y = pNode->node->Y;
	float _G = pNode->node->G;
	switch (pNode->node->dir)
	{
	case Dir::Up:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::Up, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Up, _X, _Y, pNode->node, __G));

		if (GetWall(X-1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if(JT_FindDirNode(Dir::LeftUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, __G));

		}
		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, __G));

		}


	break;
	}
	case Dir::Down:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::Down, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Down, _X, _Y, pNode->node, __G));

		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, __G));

		}

		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::Left:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::Left, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Left, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y - 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, __G));
		}
		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::LeftUp:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::LeftUp, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Up, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Up, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Left, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Left, _X, _Y, pNode->node, __G));

		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, __G));

		}
		if (GetWall(X, Y + 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::LeftDown:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::LeftDown, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Down, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Down, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Left, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Left, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y - 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::Right:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::Right, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Right, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y-1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::RightUp:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::RightUp, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Up, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Up, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Right, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Right, _X, _Y, pNode->node, __G));

		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::RightDown:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::RightDown, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Down, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Down, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::Right, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::Right, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y-1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RightUp, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LeftDown, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, __G));
		}
	}
		break;
	default:
		break;
	}
}

bool CAstar::JT_FindDirNode(GridColor::Dir _dir, int& X, int& Y, float& _G)
{
	switch (_dir)
	{
	case Dir::Up:
	{
		while (1)
		{
			Y--;

			if (GetWall(X, Y))
				break;
			//검색 했음
			_G += HV;

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X+1, Y) && !GetWall(X+1, Y-1)) ||
				(GetWall(X-1, Y) && !GetWall(X-1, Y-1)))
			{
				//coner
				return true;
			}
		}
	}
		break;
	case Dir::Down:
	{
		while (1)
		{
			Y++;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X-1, Y) && !GetWall(X-1, Y+1)) ||
				(GetWall(X+1, Y) && !GetWall(X+1, Y+1)))
			{
				//coner
				return true;
			}

		}
	}
		break;
	case Dir::Left:
	{
		while (1)
		{
			X--;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X, Y-1) && !GetWall(X-1, Y-1)) ||
				(GetWall(X, Y+1) && !GetWall(X-1, Y+1)))
			{
				//coner
				return true;
			}
		}
	}
		break;
	case Dir::Right:
	{
		while (1)
		{
			X++;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X, Y-1) && !GetWall(X+1, Y-1)) ||
				(GetWall(X, Y+1) && !GetWall(X+1, Y+1)))
			{
				//coner
				return true;
			}
		}
	}
		break;
	case Dir::LeftUp:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		while (1)
		{
			X--; Y--;
			if (GetWall(X, Y))
				break;
			_G += D;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X+1, Y) && !GetWall(X+1, Y-1)) ||
				(GetWall(X, Y+1) && !GetWall(X-1, Y+1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Left, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Up, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::RightUp:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		while (1)
		{
			X++; Y--;
			if (GetWall(X, Y))
				break;
			_G += D;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X, Y+1) && !GetWall(X+1, Y+1)) ||
				(GetWall(X-1, Y) && !GetWall(X-1, Y-1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Right, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Up, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::LeftDown:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		while (1)
		{
			X--; Y++;
			if (GetWall(X, Y))
				break;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			_G += D;
			if ((GetWall(X, Y-1) && !GetWall(X-1, Y-1)) ||
				(GetWall(X+1, Y) && !GetWall(X+1, Y-1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Left, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Down, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::RightDown:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		while (1)
		{
			X++; Y++;
			if (GetWall(X, Y))
				break;
			_G += D;
			//검색 했음

			if (X == m_iEndX && Y == m_iEndY)
				return true;

			if ((GetWall(X, Y-1) && !GetWall(X+1, Y-1)) ||
				(GetWall(X-1, Y) && !GetWall(X-1, Y+1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Right, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::Down, _X, _Y, __G))
				return true;
		}
	}
		break;
	default:
		break;
	}
	return false;
}

void CAstar::JT_CreateStartNode(MAP_STAT_NODE* pNode)
{
	// 시작 할때 한번 만 하기 때문에 CreateGridNode 에서 nullptr 이 뜨지 않는다
	int _X, _Y;
	float _G;

	_X = m_iStartX;_Y = m_iStartY;_G = 0;
	if(JT_FindDirNode(Dir::Up, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::Up, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::Down, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::Down, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::Left, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::Left, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::LeftUp, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::LeftUp, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::LeftDown, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::LeftDown, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::Right, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::Right, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::RightUp, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::RightUp, _X, _Y, pNode->node, _G));

	_X = m_iStartX; _Y = m_iStartY; _G = 0;
	if (JT_FindDirNode(Dir::RightDown, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::RightDown, _X, _Y, pNode->node, _G));
}

void CAstar::ListClear()
{
	OpenList.clear();
	CloseList.clear();
}
