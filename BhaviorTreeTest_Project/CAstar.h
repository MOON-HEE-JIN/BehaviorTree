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


namespace GridColor
{
	enum Dir
	{
		Left,
		Right,
		Up,
		Down,
		LeftUp,
		LeftDown,
		RightUp,
		RightDown
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
	virtual ~CAstarGrid();
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
	int m_iMapWidth = 0;
	int m_iMapHeight = 0;

	int m_iStartX = 0, m_iStartY = 0;
	int m_iEndX = 0, m_iEndY = 0;

	std::vector<std::vector<MAP_STAT_NODE>> m_vecMapStat;
};


class CAstar : public CAstarGrid
{
public :
	CAstar();
	CAstar(int _width, int _height, int _grid);
	~CAstar() override;

	bool FindPath(int m_iStartX, int m_iStartY, int m_iEndX, int m_iEndY, std::vector<st_Pos>& vec);

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

