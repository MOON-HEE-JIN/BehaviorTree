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
	for (int Y = 0; Y < MAP_GRID_HEIGHT; Y++)
	{
		for (int X = 0; X < MAP_GRID_WIDTH; X++)
		{
			if (m_vecMapStat[Y][X].wall || m_vecMapStat[Y][X].node == nullptr)
				continue;
			
			delete m_vecMapStat[Y][X].node;
		}
	}
}

bool CAstarGrid::GetWall(int X, int Y)
{
	if (X >= MAP_GRID_WIDTH || Y >= MAP_GRID_HEIGHT ||
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
	return static_cast<float>(abs(End_X - X) + abs(End_Y - Y));
}


void CAstarGrid::ClearAstarNode()
{
	for (int Y = 0; Y < MAP_GRID_HEIGHT; Y++)
	{
		for (int X = 0; X < MAP_GRID_WIDTH; X++)
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
	MAP_GRID_WIDTH = world_width / ASTAR_GRID_SIZE;
	MAP_GRID_HEIGHT = world_height / ASTAR_GRID_SIZE;

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
	m_vecMapStat.resize(MAP_GRID_HEIGHT);
	for (int i = 0; i < MAP_GRID_HEIGHT; i++)
		m_vecMapStat[i].resize(MAP_GRID_WIDTH);
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

	End_X = e_X; End_Y = e_Y;
	start_X = s_X; start_Y = s_Y;

	float H = GetH(start_X, start_Y);

	//PathFinder Start Node
	GridNode* pNode = m_vecMapStat[start_Y][start_X].node;

	ListClear();

	if (pNode == nullptr)
	{
		m_vecMapStat[start_Y][start_X].node = new GridNode(Dir::LL, start_X, start_Y, 0, H, H, nullptr);
	}
	else
	{
		pNode->Clear();
		pNode->dir = Dir::LL;
		pNode->G = 0;
		pNode->H = H;
		pNode->F = H;
		pNode->parent = nullptr;
	}

	// PathFinder 시작 노드 기준으로 8 방향 탐색 하여 OpenList 에 추가
	JT_CreateStartNode(&m_vecMapStat[start_Y][start_X]);

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

		if (pNode->node->X == End_X && pNode->node->Y == End_Y)
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
	vec.push_back(st_Pos(End_X, End_Y));

	GridNode* pNode = m_vecMapStat[End_Y][End_X].node;
	
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
		InsertOpenList(CreateGridNode(Dir::DD, X, Y + 1, pNode->node, HV));
	
	if (!GetWall(X, Y - 1))
		InsertOpenList(CreateGridNode(Dir::UU, X, Y - 1, pNode->node, HV));
	
	if (!GetWall(X + 1, Y + 1))
		InsertOpenList(CreateGridNode(Dir::RD, X + 1, Y + 1, pNode->node, D));
	
	if (!GetWall(X + 1, Y - 1))
		InsertOpenList(CreateGridNode(Dir::RU, X + 1, Y - 1, pNode->node, D));
	
	if (!GetWall(X + 1, Y))
		InsertOpenList(CreateGridNode(Dir::RR, X + 1, Y, pNode->node, HV));
	
	if (!GetWall(X - 1, Y + 1))
		InsertOpenList(CreateGridNode(Dir::LD, X - 1, Y + 1, pNode->node, D));
	
	if (!GetWall(X - 1, Y - 1))
		InsertOpenList(CreateGridNode(Dir::LU, X - 1, Y - 1, pNode->node, D));
	
	if (!GetWall(X - 1, Y))
		InsertOpenList(CreateGridNode(Dir::LL, X - 1, Y, pNode->node, HV));
	
}

void CAstar::JT_SerchCreateNode(MAP_STAT_NODE* pNode)
{
	int X = pNode->node->X;
	int Y = pNode->node->Y;
	float _G = pNode->node->G;
	switch (pNode->node->dir)
	{
	case Dir::UU:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::UU, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::UU, _X, _Y, pNode->node, __G));
		
		if (GetWall(X-1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if(JT_FindDirNode(Dir::LU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, __G));
			
		}
		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, __G));
			
		}
		
	
	break;
	}
	case Dir::DD:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::DD, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::DD, _X, _Y, pNode->node, __G));
		
		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, __G));
			
		}
		
		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::LL:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::LL, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LL, _X, _Y, pNode->node, __G));
		
		if (GetWall(X, Y - 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, __G));
		}
		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, __G));	
		}
	}
		break;
	case Dir::LU:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::LU, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, __G));
		
		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::UU, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::UU, _X, _Y, pNode->node, __G));
		
		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::LL, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LL, _X, _Y, pNode->node, __G));
		
		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, __G));
			
		}
		if (GetWall(X, Y + 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::LD:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::LD, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::DD, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::DD, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::LL, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::LL, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y - 1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X + 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::RR:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::RR, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RR, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y-1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::RU:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::RU, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::UU, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::UU, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::RR, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RR, _X, _Y, pNode->node, __G));

		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X, Y+1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, __G));
		}
	}
		break;
	case Dir::RD:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		if (JT_FindDirNode(Dir::RD, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::DD, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::DD, _X, _Y, pNode->node, __G));

		_X = X, _Y = Y, __G = _G;
		if (JT_FindDirNode(Dir::RR, _X, _Y, __G))
			InsertOpenList(CreateGridNode(Dir::RR, _X, _Y, pNode->node, __G));

		if (GetWall(X, Y-1))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::RU, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, __G));
		}

		if (GetWall(X - 1, Y))
		{
			_X = X, _Y = Y, __G = _G;
			if (JT_FindDirNode(Dir::LD, _X, _Y, __G))
				InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, __G));
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
	case Dir::UU:
	{
		while (1)
		{
			Y--;
			
			if (GetWall(X, Y))
				break;
			//검색 했음
			_G += HV;
			
			if (X == End_X && Y == End_Y)
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
	case Dir::DD:
	{
		while (1)
		{
			Y++;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음
			
			if (X == End_X && Y == End_Y)
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
	case Dir::LL:
	{
		while (1)
		{
			X--;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음

			if (X == End_X && Y == End_Y)
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
	case Dir::RR:
	{
		while (1)
		{
			X++;
			if (GetWall(X, Y))
				break;
			_G += HV;
			//검색 했음
			
			if (X == End_X && Y == End_Y)
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
	case Dir::LU:
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

			if (X == End_X && Y == End_Y)
				return true;

			if ((GetWall(X+1, Y) && !GetWall(X+1, Y-1)) ||
				(GetWall(X, Y+1) && !GetWall(X-1, Y+1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::LL, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::UU, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::RU:
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

			if (X == End_X && Y == End_Y)
				return true;

			if ((GetWall(X, Y+1) && !GetWall(X+1, Y+1)) ||
				(GetWall(X-1, Y) && !GetWall(X-1, Y-1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::RR, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::UU, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::LD:
	{
		int _X = X, _Y = Y;
		float __G = _G;
		while (1)
		{
			X--; Y++;
			if (GetWall(X, Y))
				break;
			//검색 했음
			
			if (X == End_X && Y == End_Y)
				return true;

			_G += D;
			if ((GetWall(X, Y-1) && !GetWall(X-1, Y-1)) ||
				(GetWall(X+1, Y) && !GetWall(X+1, Y-1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::LL, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::DD, _X, _Y, __G))
				return true;
		}
	}
		break;
	case Dir::RD:
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

			if (X == End_X && Y == End_Y)
				return true;

			if ((GetWall(X, Y-1) && !GetWall(X+1, Y-1)) ||
				(GetWall(X-1, Y) && !GetWall(X-1, Y+1)))
			{
				//coner
				return true;
			}
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::RR, _X, _Y, __G))
				return true;
			_X = X; _Y = Y; __G = _G;
			if (JT_FindDirNode(Dir::DD, _X, _Y, __G))
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
	
	_X = start_X;_Y = start_Y;_G = 0;
	if(JT_FindDirNode(Dir::UU, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::UU, _X, _Y, pNode->node, _G));
		
	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::DD, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::DD, _X, _Y, pNode->node, _G));
		
	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::LL, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::LL, _X, _Y, pNode->node, _G));
		
	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::LU, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::LU, _X, _Y, pNode->node, _G));
		
	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::LD, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::LD, _X, _Y, pNode->node, _G));

	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::RR, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::RR, _X, _Y, pNode->node, _G));

	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::RU, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::RU, _X, _Y, pNode->node, _G));

	_X = start_X; _Y = start_Y; _G = 0;
	if (JT_FindDirNode(Dir::RD, _X, _Y, _G))
		InsertOpenList(CreateGridNode(Dir::RD, _X, _Y, pNode->node, _G));
}

void CAstar::ListClear()
{
	OpenList.clear();
	CloseList.clear();
}
