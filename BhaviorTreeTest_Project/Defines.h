#pragma once

#include <math.h>

enum eSTAT_TYPE
{
	NONE,
	WALL,
	WINDOW,
	DOOR,
};

enum eDirection
{
	NORTH,
	SOUTH,
	WEST,
	EAST
};

enum class eTeam
{
	NONE,
	RED,
	BLUE,
};

enum class eState
{
	NONE,
	SPAWN,
	IDLE,
	COMBAT,
	OCCUPY,
	END,
};

enum class eCBlackBoardStatus
{
	NONE,
	Combat,
	Occupy,
	RunAway,
};

enum eTUpdateType
{
	Object,
	CoolTime,
	DuringTime,
	End,
};

enum eBTresult
{
	FAILD,
	SUCCESS,
	RUNNING,
};

enum eAstarTargetType
{
	None,
	Region,
	Pos,
};

typedef struct st_Pos
{
	int X;
	int Y;
	st_Pos() { X = 0; Y = 0; }
	st_Pos(int x, int y) { X = x; Y = y; }

	bool operator ==(st_Pos& pos) {
		if (this->X == pos.X && this->Y == pos.Y) return true;
		return false;
	}

	int Length(st_Pos pos)
	{
		int dX = X - pos.X;
		int dY = Y - pos.Y;
		return static_cast<int>(sqrt(dX * dX + dY * dY));
	}

	int LengthLength(st_Pos pos)
	{
		int dX = X - pos.X;
		int dY = Y - pos.Y;
		return (dX * dX + dY * dY);
	}

}POS;

struct st_PosInfo
{
	int iType;
	st_Pos pos;
};