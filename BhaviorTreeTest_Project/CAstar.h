#pragma once

#include <list>
#include <vector>
#include <Windows.h>
#include <stack>

#include "UnionDefine.h"

//수평 수직
const int HV = 1;
//대각선
const float D = 1.5f;
const int G_C = 25;


namespace GridColor
{
	enum Dir
	{
		LL,
		RR,
		UU,
		DD,
		LU,
		LD,
		RU,
		RD
	};
}

struct GridNode
{
	GridColor::Dir dir;
	int X, Y;
	float G, H, F;
	GridNode* parent;
	GridNode()
	{
		X = 0; Y = 0;
		G = 0; H = 0; F = 0;
		parent = nullptr;
	}
	GridNode(GridColor::Dir _dir, int _X, int _Y, float _G, float _H, float _F, GridNode* _parent)
	{
		dir = _dir;
		X = _X;
		Y = _Y;
		G = _G;
		H = _H;
		F = _F;
		parent = _parent;
	}

	void Clear()
	{
		G = 0;
		H = 0;
		F = 0;
		parent = nullptr;
	}
};

struct MAP_STAT_NODE
{
	bool wall;
	GridNode* node;
	MAP_STAT_NODE()
	{
		wall = false;
		node = nullptr;
	}
};
class CAstarGrid
{
public:
	CAstarGrid();
	~CAstarGrid();
protected:
	float GetH(int X, int Y);

public:
	bool GetWall(int X, int Y);
	void SetWall(int x, int y);
	void SetWall(int x, int y, bool bWall);

	void ClearAstarNode();
	void SetMapInit(int world_width, int world_height);
private:
	void SetMapVectorInit();
protected:
	int MAP_GRID_WIDTH;
	int MAP_GRID_HEIGHT;
	
	int start_X = 0, start_Y = 0;
	int End_X = 0, End_Y = 0;
	int r_X = 0, r_Y = 0;

	std::vector<std::vector<MAP_STAT_NODE>> m_vecMapStat;
};


class CAstar : public CAstarGrid
{
public :
	CAstar();
	CAstar(int _width, int _height, int _grid);
	~CAstar();

	bool FindPath(int start_X, int start_Y, int End_X, int End_Y, std::vector<st_Pos>& vec);
	
	bool isStraight(int x1, int y1, int x2, int y2);
private:
	void listSort();
	void ResultPath(std::vector<st_Pos>& vec);

	void JT_CreateAroundNode(MAP_STAT_NODE* pNode);
	void JT_SerchCreateNode(MAP_STAT_NODE* pNode);
	bool JT_FindDirNode(GridColor::Dir _dir, int& X, int& Y, float& _G);
	void JT_CreateStartNode(MAP_STAT_NODE* pNode);
	MAP_STAT_NODE* CreateGridNode(GridColor::Dir _dir, int _X, int _Y, GridNode* pParent, float _G);
	void InsertOpenList(MAP_STAT_NODE* pNode);
private:
	void ListClear();
private:
	std::list<MAP_STAT_NODE*> OpenList;
	std::list<MAP_STAT_NODE*> CloseList;
};

